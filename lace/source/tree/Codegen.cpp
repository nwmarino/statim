//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Codegen.h"
#include "lace/core/Diagnostics.h"
#include "lace/tree/VisitorBase.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Type.h"

#include "lir/graph/CFG.h"
#include "lir/graph/Global.h"
#include "lir/graph/Type.h"

#include <cstdint>
#include <vector>

using namespace lace;

Codegen::Codegen(Context& context, lir::CFG& graph, lir::Machine& mach)
  : VisitorBase(context), m_graph(graph), m_mach(mach), m_builder(graph) {}

void Codegen::visit(FunctionDefn& node) {
    lir::Function* func = fetch(&node);
    assert(func);

    if (!node.has_body())
        return;

    FunctionInfo& info = m_funcs.at(&node);
    if (info.complete)
        return;

    assert(func->empty());

    m_func = func;

    lir::BasicBlock* entry = lir::BasicBlock::create(m_func);
    m_builder.set_insert(entry);

    for (uint32_t i = 0, e = func->num_params(); i < e; ++i) {
        lir::Parameter* param = func->get_param(i);
        lir::Type* type = param->get_type();

        if (param->hasTrait(lir::Parameter::Trait::ARet)) {
            continue;
        } else if (param->hasTrait(lir::Parameter::Trait::Byval)) {
            type = dynamic_cast<lir::PointerType*>(type)->get_pointee();
            assert(type);

            lir::Local* local = lir::Local::create(
                m_graph, 
                type, 
                param->get_name(),
                func
            );

            m_builder.build_call(get_rtf_copy(), { 
                local, 
                param, 
                lir::Integer::get(m_graph, lir::IntegerType::get(m_graph, 64), m_mach.get_type_size(type) / 8),
            });
        } else {
            lir::Local* local = lir::Local::create(
                m_graph, 
                type, 
                param->get_name(), 
                func
            );

            m_builder.build_store(param, local);
        }
    }

    node.body()->accept(*this);

    if (!m_builder.get_insert()->terminates()) {
        if (!m_func->get_type()->has_result()) {
            m_builder.build_ret();
        } else {
            log::warn("non-void function does not always return a value", 
                log::Span { m_rib->path(), node.span() });
        }
    }

    m_func = nullptr;
    m_temp = nullptr;
    m_builder.clear_insert();

    info.complete = true;
}

void Codegen::visit(VariableDefn& node) {
    if (node.is_global()) {
        lir::Global* global = fetch(&node);
        assert(global);

        if (!node.has_init())
            return;

        GlobalInfo& info = m_globals.at(&node);
        if (info.complete)
            return;

        m_vc = Valued;
        node.init()->accept(*this);
        assert(m_temp);

        lir::Constant* init = dynamic_cast<lir::Constant*>(m_temp);
        assert(init && "global initializer is not a constant!");

        global->set_initializer(init);

        info.complete = true;
    } else {
        lir::Type* type = fetch(node.type());
        lir::Local* local = lir::Local::create(
            m_graph, 
            type, 
            node.name(), 
            m_func
        );

        if (!node.has_init())
            return;

        if (m_mach.is_scalar(type)) {
            m_vc = Valued;
            node.init()->accept(*this);
            assert(m_temp);

            m_builder.build_store(m_temp, local);
        } else {
            m_vc = Addressed;
            m_place = local;
            node.init()->accept(*this);
            assert(m_temp);

            if (m_temp != local) {
                m_builder.build_call(get_rtf_copy(), {
                    local,
                    m_temp,
                    lir::Integer::get(m_graph, lir::IntegerType::get(m_graph, 64), m_mach.get_type_size(type) / 8),
                });
            }

            m_place = nullptr;
        }

        m_temp = nullptr;
    }
}

void Codegen::visit(AdapterStmt& node) {
    switch (node.kind()) 
    {
    case AdapterStmt::Kind::Definitive: {
        VariableDefn* vd = dynamic_cast<VariableDefn*>(node.defn());
        assert(vd && "cannot generate code for a non-variable adapter!");

        vd->accept(*this);
        break;
    }

    case AdapterStmt::Kind::Expressive:
        m_vc = Valued;
        node.expr()->accept(*this);
        break;
    }

    m_temp = nullptr;
}

void Codegen::visit(IfStmt& node) {
    m_vc = Valued;
    node.condition()->accept(*this);
    assert(m_temp);

    // An 'if' condition is a boolean context, so if it isn't already, try
    // to get a boolean out of the condition value.
    m_temp = inject_comparison(m_temp);

    lir::BasicBlock* then_bb = lir::BasicBlock::create(m_func);
    lir::BasicBlock* else_bb = nullptr;
    lir::BasicBlock* merge_bb = lir::BasicBlock::create();

    if (node.has_else()) {
        else_bb = lir::BasicBlock::create();

        m_builder.build_brif(m_temp, then_bb, else_bb);
    } else {
        m_builder.build_brif(m_temp, then_bb, merge_bb);
    }

    m_builder.set_insert(then_bb);
    node.then_body()->accept(*this);

    if (!m_builder.get_insert()->terminates())
        m_builder.build_jump(merge_bb);

    if (node.has_else()) {
        m_func->append(else_bb);
        m_builder.set_insert(else_bb);
        node.else_body()->accept(*this);

        if (!m_builder.get_insert()->terminates())
            m_builder.build_jump(merge_bb);
    }

    if (merge_bb->has_preds()) {
        m_func->append(merge_bb);
        m_builder.set_insert(merge_bb);
    } else {
        delete merge_bb;
    }

    m_temp = nullptr;
}

void Codegen::visit(RestartStmt& node) {
    if (!m_builder.get_insert()->terminates()) {
        assert(m_cond && "no condition block to restart to!");
        m_builder.build_jump(m_cond);
    }

    m_temp = nullptr;
}

void Codegen::visit(RetStmt& node) {
    if (!node.has_expr()) {
        m_builder.build_ret();
        m_temp = nullptr;
        return;
    }

    lir::Type* type = fetch(node.expr()->type());
    if (m_mach.is_scalar(type)) {
        m_vc = Valued;
        node.expr()->accept(*this);
        assert(m_temp);

        m_builder.build_ret(m_temp);
    } else {
        assert(m_func->hasAggregateReturn());

        lir::Parameter* aret = m_func->get_param(0);
        assert(aret->hasTrait(lir::Parameter::Trait::ARet));

        m_vc = Addressed;
        node.expr()->accept(*this);
        assert(m_temp);

        lir::Function* copy = get_rtf_copy();

        m_builder.build_call(get_rtf_copy(), { 
            aret, 
            m_temp, 
            lir::Integer::get(m_graph, copy->get_param(2)->get_type(), m_mach.get_type_size(type) / 8) 
        });

        m_builder.build_ret();
    }

    m_temp = nullptr;
}

void Codegen::visit(RuneStmt& node) {
    switch (node.rune()->kind()) 
    {
    case Rune::Kind::Abort: {
        lir::Function* func = get_or_create_function("__abort");
        assert(func);

        m_builder.build_call(func);
        break;
    }

    case Rune::Kind::Unreachable: {
        lir::Function* func = get_or_create_function("__unreachable");
        assert(func);

        m_builder.build_call(func);
        break;
    }

    default:
        assert(false && "invalid rune statement!");
    }

    m_temp = nullptr;
}

void Codegen::visit(StopStmt& node) {
    if (!m_builder.get_insert()->terminates()) {
        assert(m_merge && "no merge block to stop to!");
        m_builder.build_jump(m_merge);
    }

    m_temp = nullptr;
}

void Codegen::visit(UntilStmt& node) {
    lir::BasicBlock* cond_bb = lir::BasicBlock::create(m_func);
    lir::BasicBlock* body_bb = nullptr;
    lir::BasicBlock* merge_bb = lir::BasicBlock::create();

    m_builder.build_jump(cond_bb);

    m_builder.set_insert(cond_bb);
    node.condition()->accept(*this);
    assert(m_temp);

    m_temp = inject_comparison(m_temp);

    if (node.has_body()) {
        body_bb = lir::BasicBlock::create(m_func);
        m_builder.build_brif(m_temp, merge_bb, body_bb);

        m_builder.set_insert(body_bb);

        lir::BasicBlock* prev_cond = m_cond;
        lir::BasicBlock* prev_merge = m_merge;
        m_cond = cond_bb;
        m_merge = merge_bb;

        node.body()->accept(*this);

        if (!m_builder.get_insert()->terminates())
            m_builder.build_jump(cond_bb);

        m_cond = prev_cond;
        m_merge = prev_merge;
    } else {
        m_builder.build_brif(m_temp, merge_bb, cond_bb);
    }

    m_func->append(merge_bb);
    m_builder.set_insert(merge_bb);
    m_temp = nullptr;
}

void Codegen::visit(BoolLiteral& node) {
    assert(m_vc == Valued);

    m_temp = lir::Integer::get(
        m_graph, lir
        ::IntegerType::get(m_graph, 8), 
        static_cast<int64_t>(node.get_value())
    );
}

void Codegen::visit(IntegerLiteral& node) {
    assert(m_vc == Valued);

    m_temp = lir::Integer::get(m_graph, fetch(node.type()), node.get_value());
}

void Codegen::visit(CharLiteral& node) {
    assert(m_vc == Valued);

    m_temp = lir::Integer::get(
        m_graph, 
        lir::IntegerType::get(m_graph, 8), 
        static_cast<int64_t>(node.get_value())
    );
}

void Codegen::visit(FloatLiteral& node) {
    assert(m_vc == Valued);

    m_temp = m_builder.build_const(lir::Float::get(
        m_graph, 
        fetch(node.type()), 
        node.get_value())
    );
}

void Codegen::visit(NullLiteral& node) {
    assert(m_vc == Valued);
    
    m_temp = lir::Null::get(m_graph, fetch(node.type()));
}

void Codegen::visit(StringLiteral& node) {
    assert(m_vc == Valued);

    m_temp = m_builder.build_string(lir::String::get(m_graph, node.value()));
}

void Codegen::visit(BinaryOp& node) {
    switch (node.op())
    {
    case BinaryOp::Assign:
        return codegen_assignment(node);
    case BinaryOp::Add:
    case BinaryOp::Sub:
        return codegen_addition(node);
    case BinaryOp::Mul:
        return codegen_multiply(node);
    case BinaryOp::Div:
    case BinaryOp::Mod:
        return codegen_division(node);
    case BinaryOp::And:
    case BinaryOp::Or:
    case BinaryOp::Xor:
        return codegen_bitwise_arithmetic(node);
    case BinaryOp::LShift:
    case BinaryOp::RShift:
        return codegen_bitwise_shift(node);
    case BinaryOp::LogicAnd:
        return codegen_logical_and(node);
    case BinaryOp::LogicOr:
        return codegen_logical_or(node);
    case BinaryOp::Eq:
    case BinaryOp::NEq:
    case BinaryOp::Lt:
    case BinaryOp::LtEq:
    case BinaryOp::Gt:
    case BinaryOp::GtEq:
        return codegen_comparison(node);
    default:
        assert(false && "invalid binary operator!");
    }
}

void Codegen::visit(UnaryOp& node) {
    switch (node.op())
    {
    case UnaryOp::Negate:
        return codegen_negation(node);
    case UnaryOp::Not:
        return codegen_bitwise_not(node);
    case UnaryOp::LogicNot:
        return codegen_logical_not(node);
    case UnaryOp::AddressOf:
        return codegen_address_of(node);
    case UnaryOp::Dereference:
        return codegen_dereference(node);
    default:
        assert(false && "invalid unary operator!");
    }
}

void Codegen::visit(AccessExpr& node) {
    ValueContext vc = m_vc;

    lir::Value* ptr = nullptr;
    if (dynamic_cast<PointerType*>(node.base()->type())) {
        // If this access is functionally similar to C-style '->' access, then
        // we need to load the base to get at the underlying structure.
        m_vc = Valued;
    } else if (dynamic_cast<StructType*>(node.base()->type())) {
        m_vc = Addressed;
    } else {
        assert(false && "invalid type operand to access!");
    }

    node.base()->accept(*this);
    assert(m_temp);
    ptr = m_temp;

    lir::Type* type = lir::PointerType::get(m_graph, fetch(node.type()));
    assert(node.is_resolved());

    if (FieldDefn* field = dynamic_cast<FieldDefn*>(node.field())) {
        m_temp = m_builder.build_access(type, ptr, lir::Integer::get(
            m_graph, 
            lir::IntegerType::get(m_graph, 64), 
            field->get_index()
        ));

        if (vc == Valued)
            m_temp = m_builder.build_load(fetch(node.type()), m_temp);
    } else if (FunctionDefn* method = dynamic_cast<FunctionDefn*>(node.field())) {
        m_temp = fetch(method);

        assert(m_temp);
        assert(vc == Addressed && "cannot generate value from a function pointer!");
    } else {
        assert(false && "invalid access field type!");   
    }
}

void Codegen::visit(CallExpr& node) {
    m_vc = Addressed;
    node.callee()->accept(*this);
    lir::Value* callee = m_temp;
    assert(callee);

    std::vector<lir::Value*> args = {};
    args.reserve(node.num_args());

    lir::Value* aret = nullptr;
    lir::Type* rt = fetch(node.type());
    if (!m_mach.is_scalar(rt)) {
        if (m_place) {
            aret = m_place;
        } else {
            aret = lir::Local::create(
                m_graph, 
                rt, 
                std::to_string(m_graph.get_def_id()),
                m_func
            );
        }
        
        args.push_back(aret);
    }

    if (node.has_receiver()) {
        lir::Value* receiver = nullptr;
        if (dynamic_cast<const PointerType*>(node.receiver()->type())) {
            m_vc = Valued;
        } else if (dynamic_cast<StructType*>(node.receiver()->type())) {
            m_vc = Addressed;   
        }

        node.receiver()->accept(*this);
        assert(m_temp);
        receiver = m_temp;

        args.push_back(receiver);
    }

    for (Expr* arg : node.args()) {
        if (!m_mach.is_scalar(fetch(arg->type()))) {
            m_vc = Addressed;
        } else {
            m_vc = Valued;
        }

        arg->accept(*this);
        assert(m_temp);

        args.push_back(m_temp);
    }

    lir::Call* call = m_builder.build_call(dynamic_cast<lir::Function*>(callee), args);
    
    if (aret) {
        m_temp = aret;
    } else {
        m_temp = call;
    }
}

void Codegen::visit(CastExpr& node) {
    assert(m_vc == Valued);

    node.expr()->accept(*this);
    assert(m_temp);

    lir::Type* source = m_temp->get_type();
    lir::Type* dest = fetch(node.type());

    if (source->is_integer_type()) {
        if (dest->is_integer_type()) {
            // Handle integer -> integer type casts.
            if (lir::Integer* integer = dynamic_cast<lir::Integer*>(m_temp)) {
                // @Todo: check if the constant integer value actually fits 
                // within the destination type. Or, let the backend handle it
                // as undefined behaviour.
                m_temp = lir::Integer::get(m_graph, dest, integer->get_value());
                return;
            }

            const uint32_t source_size = m_mach.get_type_size(source);
            const uint32_t dest_size = m_mach.get_type_size(dest);

            if (source_size == dest_size) {
                return;
            } else if (source_size > dest_size) {
                m_temp = m_builder.build_itrunc(dest, m_temp);
                return;
            }
            
            if (node.expr()->type()->is_signed_integer()) {
                m_temp = m_builder.build_sext(dest, m_temp);
            } else {
                m_temp = m_builder.build_zext(dest, m_temp);
            }

            return;
        } else if (dest->is_float_type()) {
            // Handle integer -> floating point type casts.
            if (lir::Integer* integer = dynamic_cast<lir::Integer*>(m_temp)) {
                m_temp = m_builder.build_const(lir::Float::get(
                    m_graph, 
                    dest, 
                    integer->get_value())
                );
                return;
            }

            if (node.expr()->type()->is_signed_integer()) {
                m_temp = m_builder.build_s2f(dest, m_temp);
            } else {
                m_temp = m_builder.build_u2f(dest, m_temp);
            }

            return;
        } else if (dest->is_pointer_type()) {
            // Handle integer -> pointer type casts.
            if (lir::Integer* integer = dynamic_cast<lir::Integer*>(m_temp)) {
                // Fold cast<*T>(0) to null.
                if (integer->get_value() == 0) {
                    m_temp = lir::Null::get(m_graph, dest);
                    return;
                }
            }

            m_temp = m_builder.build_i2p(dest, m_temp);
            return;
        }
    } else if (source->is_float_type()) {
        if (dest->is_integer_type()) {
            // Handle floating point -> integer type casts.
            if (lir::Float* fp = dynamic_cast<lir::Float*>(m_temp)) {
                m_temp = lir::Integer::get(m_graph, dest, fp->get_value());
                return;
            }

            if (node.type()->is_signed_integer()) {
                m_temp = m_builder.build_f2s(dest, m_temp);
            } else {
                m_temp = m_builder.build_f2u(dest, m_temp);
            }

            return;
        } else if (dest->is_float_type()) {
            // Handle floating point -> floating point type casts.
            if (lir::Float* fp = dynamic_cast<lir::Float*>(m_temp)) {
                m_temp = lir::Float::get(m_graph, dest, fp->get_value());
                return;
            }

            const uint32_t source_size = m_mach.get_type_size(source);
            const uint32_t dest_size = m_mach.get_type_size(dest);

            if (source_size == dest_size) {
                return;
            } else if (source_size > dest_size) {
                m_temp = m_builder.build_ftrunc(dest, m_temp);
            } else {
                m_temp = m_builder.build_fext(dest, m_temp);
            }

            return;
        }
    } else if (source->is_array_type()) {
        if (dest->is_pointer_type()) {
            m_temp = m_builder.build_reint(dest, m_temp);
            return;
        }
    } else if (source->is_pointer_type()) {
        if (dest->is_integer_type()) {
            // Handle pointer -> integer type casts.
            if (lir::Null* null = dynamic_cast<lir::Null*>(m_temp)) {
                // Fold null to 0s.
                m_temp = lir::Integer::get_zero(m_graph, dest);
            } else {
                m_temp = m_builder.build_p2i(dest, m_temp);
            }

            return;
        } else if (dest->is_pointer_type()) {
            // Handle pointer -> pointer type casts.
            if (dynamic_cast<lir::Null*>(m_temp)) {
                m_temp = lir::Null::get(m_graph, dest);
            } else {
                m_temp = m_builder.build_reint(dest, m_temp);
            }
            
            return;
        }
    }
    
    assert(false && "invalid type cast!");
}

void Codegen::visit(ParenExpr& node) {
    node.expr()->accept(*this);
}

void Codegen::visit(RefExpr& node) {
    assert(node.is_resolved());

    if (FunctionDefn* fd = dynamic_cast<FunctionDefn*>(node.defn())) {
        m_temp = fetch(fd);
        assert(m_temp);
        assert(m_vc == Addressed && "cannot generate value from a function pointer!");
    } else if (ParameterDefn* pd = dynamic_cast<ParameterDefn*>(node.defn())) {
        assert(m_func && "cannot resolve parameter outside of a function!");

        lir::Local* local = m_func->get_local(pd->name());
        assert(local && "local does not exist for parameter!");

        m_temp = local;
    } else if (VariableDefn* vd = dynamic_cast<VariableDefn*>(node.defn())) {
        if (vd->is_global()) {
            m_temp = fetch(vd);
            assert(m_temp);
        } else {
            assert(m_func && "cannot resolve local variable outside of a function!");

            lir::Local* local = m_func->get_local(vd->name());
            assert(local && "local does not exist for local variable!");

            m_temp = local;
        }
    } else if (VariantDefn* vd = dynamic_cast<VariantDefn*>(node.defn())) {
        assert(m_vc == Valued && "cannot generate address for an enum constant!");

        m_temp = lir::Integer::get(m_graph, fetch(node.type()), vd->get_value());
        return;
    }

    if (m_vc == Valued)
        m_temp = m_builder.build_load(fetch(node.type()), m_temp);
}

void Codegen::visit(SizeofExpr& node) {
    assert(m_vc == Valued);

    m_temp = lir::Integer::get(
        m_graph,
        fetch(node.type()),
        (m_mach.get_type_size(fetch(node.target())) / 8)
    );
}

void Codegen::visit(StructInitExpr& node) {
    assert(m_vc == Addressed);

    lir::Value* dest = nullptr;
    lir::Type* type = fetch(node.type());
    
    StructType* st = dynamic_cast<StructType*>(node.type());
    assert(st);

    StructDefn* sd = st->defn();
    assert(sd);

    if (m_place) {
        dest = m_place;
    } else {
        dest = lir::Local::create(
            m_graph, 
            type, 
            std::to_string(m_graph.get_def_id()),
            m_func
        );
    }

    for (uint32_t i = 0; i < sd->num_fields(); ++i) {
        FieldDefn* field = sd->get_field(i);
        Expr* init = nullptr;

        for (FieldInitExpr* fi : node.fields()) {
            if (fi->field() == field) {
                init = fi->expr();
                break;
            }
        }

        if (!init) {
            if (!field->has_init())
                continue;

            init = field->init();
        }

        lir::Type* ft = fetch(field->type());

        lir::Value* ptr = m_builder.build_access(
            lir::PointerType::get(m_graph, ft), 
            dest, 
            lir::Integer::get(m_graph, lir::IntegerType::get(m_graph, 64), field->get_index())
        );

        if (m_mach.is_scalar(ft)) {
            m_vc = Valued;
            init->accept(*this);
            assert(m_temp);

            m_builder.build_store(m_temp, ptr);
        } else {
            lir::Value* prev_place = m_place;
            m_place = ptr;

            m_vc = Addressed;
            init->accept(*this);
            assert(m_temp);

            if (m_temp != ptr) {
                m_builder.build_call(get_rtf_copy(), {
                    ptr,
                    m_temp,
                    lir::Integer::get(m_graph, lir::IntegerType::get(m_graph, 64), m_mach.get_type_size(ft) / 8)
                });
            }

            m_place = prev_place;
        }
    }

    m_temp = dest;
}

void Codegen::visit(SubscriptExpr& node) {
    ValueContext vc = m_vc;

    lir::Value* ptr = nullptr;
    if (dynamic_cast<PointerType*>(node.base()->type())) {
        m_vc = Valued;
        node.base()->accept(*this);
        assert(m_temp);
        ptr = m_temp;
    } else {
        assert(false && "invalid type operand to subscript!");
    }

    m_vc = Valued;
    node.index()->accept(*this);
    assert(m_temp);

    m_temp = m_builder.build_offptr(
        lir::PointerType::get(m_graph, fetch(node.type())), 
        ptr,
        m_temp
    );

    if (vc == Valued)
        m_temp = m_builder.build_load(fetch(node.type()), m_temp);
}

std::string Codegen::mangle(NamedDefn* defn) {
    assert(defn && "defn cannot be null!");

    std::string prefix = defn->rib()->name();
    std::size_t pos = 0;

    while ((pos = prefix.find("::", pos)) != std::string::npos) {
        prefix.replace(pos, 2, ".");
        pos++;
    }

    FunctionDefn* fd = dynamic_cast<FunctionDefn*>(defn);
    if (fd && fd->has_receiver())
        prefix += '.' + fd->get_receiver_type()->string();

    return prefix + '.' + defn->name();
}

lir::Function* Codegen::fetch(FunctionDefn* defn) {
    assert(defn && "defn cannot be null!");

    // Check if the function has been lowered already, and return it if so.
    auto it = m_funcs.find(defn);
    if (it != m_funcs.end())
        return it->second.func;
    
    lower(defn);

    it = m_funcs.find(defn);
    assert(it != m_funcs.end() && "function lowered, but unavailable!");

    return it->second.func;
}

lir::Global* Codegen::fetch(VariableDefn* defn) {
    assert(defn && "defn cannot be null!");
    assert(defn->is_global() && "variable is not a global!");

    // Check if the variable has been lowered already, and return it if so.
    auto it = m_globals.find(defn);
    if (it != m_globals.end())
        return it->second.global;
    
    lower(defn);

    it = m_globals.find(defn);
    assert(it != m_globals.end() && "variable lowered, but unavailable!");

    return it->second.global;
}

lir::StructType* Codegen::fetch(StructDefn* defn) {
    assert(defn && "defn cannot be null!");

    // Check if the structure has been lowered already, and return it if so.
    auto it = m_structs.find(defn->type());
    if (it != m_structs.end())
        return it->second;

    lower(defn);

    it = m_structs.find(defn->type());
    assert(it != m_structs.end() && "structure lowered, but unavailable!");

    return it->second;
}

lir::Type* Codegen::fetch(Type* type) {
    assert(type && "type cannot be null!");

    if (auto at = dynamic_cast<AliasType*>(type)) {
        return fetch(at->aliased());
    } else if (auto bt = dynamic_cast<BuiltinType*>(type)) {
        switch (bt->kind()) 
        {
        case BuiltinType::Kind::Void:
            return lir::VoidType::get(m_graph);
        case BuiltinType::Kind::Bool:
        case BuiltinType::Kind::Char:
        case BuiltinType::Kind::Int8:
        case BuiltinType::Kind::UInt8:
            return lir::IntegerType::get(m_graph, 8);
        case BuiltinType::Kind::Int16:
        case BuiltinType::Kind::UInt16:
            return lir::IntegerType::get(m_graph, 16);
        case BuiltinType::Kind::Int32:
        case BuiltinType::Kind::UInt32:
            return lir::IntegerType::get(m_graph, 32);
        case BuiltinType::Kind::Int64:        
        case BuiltinType::Kind::UInt64:
            return lir::IntegerType::get(m_graph, 64);
        case BuiltinType::Kind::Float32:
            return lir::FloatType::get(m_graph, 32);
        case BuiltinType::Kind::Float64:
            return lir::FloatType::get(m_graph, 64);
        }
    } else if (auto et = dynamic_cast<EnumType*>(type)) {
        return fetch(et->underlying());
    } else if (auto ft = dynamic_cast<FunctionType*>(type)) {
        std::vector<lir::Type*> params = {};
        params.reserve(ft->num_params());

        lir::Type* rt = fetch(ft->result());
        if (!m_mach.is_scalar(rt)) {
            // If the result type is an aggregate, then it must be passed as 
            // the first argument via a hidden pointer. The return type then 
            // becomes void.
            params.push_back(lir::PointerType::get(m_graph, rt));
            rt = lir::VoidType::get(m_graph);
        }

        for (uint32_t i = 0; i < ft->num_params(); ++i) {
            lir::Type* pt = fetch(ft->get_param(i));

            if (m_mach.is_scalar(pt)) {
                params.push_back(pt);
            } else {
                // If the parameter type is an aggregate, then it is passed via
                // a hidden pointer.
                params.push_back(lir::PointerType::get(m_graph, pt));
            }
        }

        return lir::FunctionType::get(m_graph, params, rt);
    } else if (PointerType* pt = dynamic_cast<PointerType*>(type)) {
        return lir::PointerType::get(m_graph, fetch(pt->pointee()));
    } else if (StructType* st = dynamic_cast<StructType*>(type)) {
        return fetch(st->defn());
    }

    assert(false && "failed to lower lace type to an LIR equivelant!");
}

lir::Value* Codegen::inject_comparison(lir::Value* value) {
    assert(value && "value cannot be null!");

    lir::Type* type = value->get_type();

    if (type->is_integer_type(8)) {
        return value;
    } else if (type->is_integer_type()) {
        return m_builder.build_cmp_ine(value, lir::Integer::get_zero(m_graph, type));
    } else if (type->is_float_type()) {
        return m_builder.build_cmp_fne(value, lir::Float::get_zero(m_graph, type));
    } else if (type->is_pointer_type()) {
        return m_builder.build_cmp_ine(value, lir::Null::get(m_graph, type));
    }

    assert(false && "value cannot be reduced to a boolean!");
}

lir::Function* Codegen::get_or_create_function(const std::string& name, 
                                               lir::Type* result,
                                               const std::vector<lir::Type*>& args) {
    if (lir::Function* func = m_graph.get_function(name))
        return func;
    
    std::vector<lir::Parameter*> params(args.size(), nullptr);
    for (uint32_t i = 0; i < args.size(); ++i)
        params[i] = lir::Parameter::create(args[i]);

    return lir::Function::create(
        m_graph, 
        lir::Function::LinkageType::Public, 
        lir::FunctionType::get(m_graph, args, result), 
        name, 
        params
    );
}

lir::Function* Codegen::get_rtf_copy() {
    return get_or_create_function("__copy", lir::VoidType::get(m_graph), {
        lir::PointerType::get(m_graph, lir::VoidType::get(m_graph)),
        lir::PointerType::get(m_graph, lir::VoidType::get(m_graph)),
        lir::IntegerType::get(m_graph, 64),
    });
}

void Codegen::lower(FunctionDefn* defn) {
    if (m_funcs.contains(defn))
        return;

    lir::Function::LinkageType linkage = lir::Function::LinkageType::Private;
    if (defn->has_rune(Rune::Kind::Public))
        linkage = lir::Function::LinkageType::Public;

    std::vector<lir::Parameter*> params = {};
    params.reserve(defn->num_params());

    lir::Type* rt = fetch(defn->get_return_type());
    if (!m_mach.is_scalar(rt)) {
        // Return type is an aggregate, so it is passed via a hidden pointer as 
        // the first parameter.
        params.push_back(lir::Parameter::create(
            lir::PointerType::get(m_graph, rt), 
            "ret.ptr", 
            lir::Parameter::Trait::ARet
        ));
    }

    if (ParameterDefn* receiver = defn->receiver()) {
        params.push_back(lir::Parameter::create(
            fetch(receiver->type()), 
            receiver->name()
        ));
    }

    for (ParameterDefn* param : defn->params()) {
        lir::Parameter::Trait trait = lir::Parameter::Trait::None;

        std::string name = param->name();
        if (name == "_") {
            // Unnamed parameters i.e. '_' should be cleared at this point. 
            name.clear();
        }

        lir::Type* type = fetch(param->type());
        assert(type);

        if (!m_mach.is_scalar(type)) {
            // Parameter type is an aggregate, so it should be passed "by value" via a hidden ptr.
            type = lir::PointerType::get(m_graph, type);
            trait = lir::Parameter::Trait::Byval;
        }

        params.push_back(lir::Parameter::create(type, name, trait));
    }

    lir::FunctionType* ft = dynamic_cast<lir::FunctionType*>(fetch(defn->type()));
    assert(ft);

    lir::Function* func = lir::Function::create(
        m_graph, 
        linkage, 
        ft,
        mangle(defn),
        params
    );

    m_funcs.emplace(defn, FunctionInfo {
        func,
        false, // incomplete (no body)
    });
}

void Codegen::lower(VariableDefn* defn) {
    assert(defn->is_global() && "variable must be a global!");
    
    if (m_globals.contains(defn))
        return;

    lir::Global::LinkageType linkage = lir::Global::LinkageType::Private;
    if (defn->has_rune(Rune::Kind::Public))
        linkage = lir::Global::LinkageType::Public;

    lir::Global* global = lir::Global::create(
        m_graph, 
        fetch(defn->type()), 
        linkage, 
        mangle(defn), 
        true
    );

    m_globals.emplace(defn, GlobalInfo {
        global,
        false, // incomplete (no init)
    });
}

void Codegen::lower(StructDefn* defn) {
    if (m_structs.contains(defn->type()))
        return;

    lir::StructType* st = lir::StructType::create(m_graph, mangle(defn), {});
    m_structs.emplace(defn->type(), st);

    for (FieldDefn* field : defn->fields()) {
        lir::Type* type = fetch(field->type());
        assert(type);

        st->append_field(type);
    }
}

void Codegen::codegen_assignment(BinaryOp& node) {
    assert(m_vc == Valued);

    m_vc = Addressed;
    node.lhs()->accept(*this);
    assert(m_temp);
    lir::Value* ptr = m_temp;
    
    lir::Type* type = fetch(node.type());

    if (m_mach.is_scalar(type)) {
        m_vc = Valued;
        node.rhs()->accept(*this);
        assert(m_temp);
    
        m_builder.build_store(m_temp, ptr);
    } else {
        m_place = ptr;
        m_vc = Addressed;

        node.rhs()->accept(*this);
        assert(m_temp);

        if (m_temp != ptr) {
            m_builder.build_call(get_rtf_copy(), {
                ptr,
                m_temp,
                lir::Integer::get(m_graph, lir::IntegerType::get(m_graph, 64), m_mach.get_type_size(type) / 8),
            });
        }

        m_place = nullptr;
    }
}

void Codegen::codegen_addition(BinaryOp& node) {
    assert(m_vc == Valued);
    assert(node.op() == BinaryOp::Add || node.op() == BinaryOp::Sub);

    node.lhs()->accept(*this);
    assert(m_temp);
    lir::Value* lhs = m_temp;

    m_vc = Valued;
    node.rhs()->accept(*this);
    assert(m_temp);
    lir::Value* rhs = m_temp;

    lir::Type* lhs_type = lhs->get_type();
    lir::Type* rhs_type = rhs->get_type();

    if (lhs_type->is_pointer_type() && rhs_type->is_integer_type()) {
        // Handle pointer arithmetic.
        if (node.op() == BinaryOp::Sub) {
            // For '-' pointer arithmetic, the index needs to be negated.
            if (lir::Integer* integer = dynamic_cast<lir::Integer*>(rhs)) {
                rhs = lir::Integer::get(
                    m_graph, 
                    rhs->get_type(), 
                    -integer->get_value()
                );
            } else {
                rhs = m_builder.build_ineg(rhs);
            }
        }

        m_temp = m_builder.build_offptr(lhs_type, lhs, rhs);
        return;
    } else if (lhs_type->is_integer_type() && rhs_type->is_integer_type()) {
        lir::Integer* lhs_integer = dynamic_cast<lir::Integer*>(lhs);
        lir::Integer* rhs_integer = dynamic_cast<lir::Integer*>(rhs);

        if (node.op() == BinaryOp::Add) {
            if (lhs_integer && rhs_integer) {
                m_temp = lir::Integer::get(
                    m_graph, 
                    lhs_type, 
                    lhs_integer->get_value() + rhs_integer->get_value()
                );
            } else {
                m_temp = m_builder.build_iadd(lhs, rhs);
            }
        } else {
            if (lhs_integer && rhs_integer) {
                m_temp = lir::Integer::get(
                    m_graph, 
                    lhs_type, 
                    lhs_integer->get_value() - rhs_integer->get_value()
                );
            } else {
                m_temp = m_builder.build_isub(lhs, rhs);
            }
        }
        return;
    } else if (lhs_type->is_float_type() && rhs_type->is_float_type()) {
        lir::Float* lhs_fp = dynamic_cast<lir::Float*>(lhs);
        lir::Float* rhs_fp = dynamic_cast<lir::Float*>(rhs);

        if (node.op() == BinaryOp::Add) {
            if (lhs_fp && rhs_fp) {
                m_temp = lir::Float::get(
                    m_graph,
                    lhs_type,
                    lhs_fp->get_value() + rhs_fp->get_value()
                );
            } else {
                m_temp = m_builder.build_fadd(lhs, rhs);
            }
        } else {
            if (lhs_fp && rhs_fp) {
                m_temp = lir::Float::get(
                    m_graph,
                    lhs_type,
                    lhs_fp->get_value() - rhs_fp->get_value()
                );
            } else {
                m_temp = m_builder.build_fsub(lhs, rhs);
            }
        }
    } else {
        assert(false && "invalid add/sub operation!");
    }
}

void Codegen::codegen_multiply(BinaryOp& node) {
    assert(m_vc == Valued);
    assert(node.op() == BinaryOp::Mul);

    node.lhs()->accept(*this);
    assert(m_temp);
    lir::Value* lhs = m_temp;

    m_vc = Valued;
    node.rhs()->accept(*this);
    assert(m_temp);
    lir::Value* rhs = m_temp;

    lir::Type* lhs_type = lhs->get_type();
    lir::Type* rhs_type = rhs->get_type();

    if (lhs_type->is_integer_type() && rhs_type->is_integer_type()) {
        lir::Integer* lhs_integer = dynamic_cast<lir::Integer*>(lhs);
        lir::Integer* rhs_integer = dynamic_cast<lir::Integer*>(rhs);

        if (lhs_integer && rhs_integer) {
            m_temp = lir::Integer::get(
                m_graph, 
                lhs_type, 
                lhs_integer->get_value() * rhs_integer->get_value()
            );
        } else {
            m_temp = m_builder.build_imul(lhs, rhs);
        }
    } else if (lhs_type->is_float_type() && rhs_type->is_float_type()) {
        lir::Float* lhs_fp = dynamic_cast<lir::Float*>(lhs);
        lir::Float* rhs_fp = dynamic_cast<lir::Float*>(rhs);

        if (lhs_fp && rhs_fp) {
            m_temp = lir::Float::get(
                m_graph,
                lhs_type,
                lhs_fp->get_value() * rhs_fp->get_value()
            );
        } else {
            m_temp = m_builder.build_fmul(lhs, rhs);
        }
    } else {
        assert(false && "invalid mul operation!");
    }
}

void Codegen::codegen_division(BinaryOp& node) {
    assert(m_vc == Valued);
    assert(node.op() == BinaryOp::Div || node.op() == BinaryOp::Mod);

    node.lhs()->accept(*this);
    assert(m_temp);
    lir::Value* lhs = m_temp;

    m_vc = Valued;
    node.rhs()->accept(*this);
    assert(m_temp);
    lir::Value* rhs = m_temp;

    lir::Type* lhs_type = lhs->get_type();
    lir::Type* rhs_type = rhs->get_type();

    if (lhs_type->is_integer_type() && rhs_type->is_integer_type()) {
        lir::Integer* lhs_integer = dynamic_cast<lir::Integer*>(lhs);
        lir::Integer* rhs_integer = dynamic_cast<lir::Integer*>(rhs);

        if (lhs_integer && rhs_integer) {
            if (node.op() == BinaryOp::Div) {
                m_temp = lir::Integer::get(
                    m_graph, 
                    lhs_type, 
                    lhs_integer->get_value() / rhs_integer->get_value()
                );
            } else if (node.op() == BinaryOp::Mod) {
                m_temp = lir::Integer::get(
                    m_graph,
                    lhs_type,
                    lhs_integer->get_value() % rhs_integer->get_value()
                );
            } else {
                assert(false && "invalid operator!");
            }

            return;
        }
 
        if (node.lhs()->type()->is_signed_integer()) {
            if (node.op() == BinaryOp::Div) {
                m_temp = m_builder.build_sdiv(lhs, rhs);
            } else if (node.op() == BinaryOp::Mod) {
                m_temp = m_builder.build_smod(lhs, rhs);
            }
        } else {
            if (node.op() == BinaryOp::Div) {
                m_temp = m_builder.build_udiv(lhs, rhs);
            } else if (node.op() == BinaryOp::Mod) {
                m_temp = m_builder.build_umod(lhs, rhs);
            }
        }

        return;
    } else if (lhs_type->is_float_type() && rhs_type->is_float_type()) {
        assert(node.op() == BinaryOp::Div && "fmod unsupported!");

        lir::Float* lhs_fp = dynamic_cast<lir::Float*>(lhs);
        lir::Float* rhs_fp = dynamic_cast<lir::Float*>(rhs);

        if (lhs_fp && rhs_fp) {
            m_temp = lir::Float::get(
                m_graph,
                lhs_type,
                lhs_fp->get_value() / rhs_fp->get_value()
            );
        } else {
            m_temp = m_builder.build_fdiv(lhs, rhs);
        }

        return;
    }

    assert(false && "invalid div/mod operation!");
}

void Codegen::codegen_bitwise_arithmetic(BinaryOp& node) {
    assert(m_vc == Valued);
    assert(node.op() == BinaryOp::And || node.op() == BinaryOp::Or || 
           node.op() == BinaryOp::Xor);

    node.lhs()->accept(*this);
    assert(m_temp);
    lir::Value* lhs = m_temp;

    m_vc = Valued;
    node.rhs()->accept(*this);
    assert(m_temp);
    lir::Value* rhs = m_temp;

    lir::Integer* lhs_integer = dynamic_cast<lir::Integer*>(lhs);
    lir::Integer* rhs_integer = dynamic_cast<lir::Integer*>(rhs);

    if (node.op() == BinaryOp::And) {
        if (lhs_integer && rhs_integer) {
            m_temp = lir::Integer::get(
                m_graph, 
                lhs->get_type(), 
                lhs_integer->get_value() & rhs_integer->get_value()
            );
        } else {
            m_temp = m_builder.build_and(lhs, rhs);
        }
    } else if (node.op() == BinaryOp::Or) {
        if (lhs_integer && rhs_integer) {
            m_temp = lir::Integer::get(
                m_graph, 
                lhs->get_type(), 
                lhs_integer->get_value() | rhs_integer->get_value()
            );
        } else {
            m_temp = m_builder.build_or(lhs, rhs);
        }
    } else if (node.op() == BinaryOp::Xor) {
        if (lhs_integer && rhs_integer) {
            m_temp = lir::Integer::get(
                m_graph, 
                lhs->get_type(), 
                lhs_integer->get_value() ^ rhs_integer->get_value()
            );
        } else {
            m_temp = m_builder.build_xor(lhs, rhs);
        }
    } else {
        assert(false && "invalid and/or/xor operation!");
    }
}

void Codegen::codegen_bitwise_shift(BinaryOp& node) {
    assert(m_vc == Valued);
    assert(node.op() == BinaryOp::LShift || node.op() == BinaryOp::RShift);

    node.lhs()->accept(*this);
    assert(m_temp);
    lir::Value* lhs = m_temp;

    m_vc = Valued;
    node.rhs()->accept(*this);
    assert(m_temp);
    lir::Value* rhs = m_temp;

    lir::Integer* lhs_integer = dynamic_cast<lir::Integer*>(lhs);
    lir::Integer* rhs_integer = dynamic_cast<lir::Integer*>(rhs);

    if (node.op() == BinaryOp::LShift) {
        if (lhs_integer && rhs_integer) {
            m_temp = lir::Integer::get(
                m_graph, 
                lhs->get_type(), 
                lhs_integer->get_value() << rhs_integer->get_value()
            );
        } else {
            m_temp = m_builder.build_shl(lhs, rhs);
        }
    } else if (node.op() == BinaryOp::RShift) {
        if (lhs_integer && rhs_integer) {
            m_temp = lir::Integer::get(
                m_graph, 
                lhs->get_type(), 
                lhs_integer->get_value() | rhs_integer->get_value()
            );

            return;
        }
        
        if (node.lhs()->type()->is_signed_integer()) {
            m_temp = m_builder.build_sar(lhs, rhs); 
        } else {
            m_temp = m_builder.build_shr(lhs, rhs);
        }
    } else {
        assert(false && "invalid ls/rs operation!");
    }
}

void Codegen::codegen_comparison(BinaryOp& node) {
    assert(m_vc == Valued);

    node.lhs()->accept(*this);
    assert(m_temp);
    lir::Value *lhs = m_temp;

    m_vc = Valued;
    node.rhs()->accept(*this);
    assert(m_temp);
    lir::Value* rhs = m_temp;

    // @Todo: Implement constant folding here.

    Type* type = node.lhs()->type();
    switch (node.op()) 
    {
    case BinaryOp::Eq:
        if (type->is_integer() || dynamic_cast<const PointerType*>(type)) {
            m_temp = m_builder.build_cmp_ieq(lhs, rhs);
        } else if (type->is_floating_point()) {
            m_temp = m_builder.build_cmp_feq(lhs, rhs);
        } else {
            break;
        }

        return;

    case BinaryOp::NEq:
        if (type->is_integer() || dynamic_cast<const PointerType*>(type)) {
            m_temp =m_builder.build_cmp_ine(lhs, rhs);
        } else if (type->is_floating_point()) {
            m_temp = m_builder.build_cmp_fne(lhs, rhs);
        } else {
            break;
        }

        return;

    case BinaryOp::Lt:
        if (type->is_signed_integer() || dynamic_cast<const PointerType*>(type)) {
            m_temp = m_builder.build_cmp_slt(lhs, rhs);
        } else if (type->is_unsigned_integer()) {
            m_temp = m_builder.build_cmp_ult(lhs, rhs);
        } else if (type->is_floating_point()) {
            m_temp = m_builder.build_cmp_flt(lhs, rhs);
        } else {
            break;
        }

        return;

    case BinaryOp::LtEq:
        if (type->is_signed_integer() || dynamic_cast<const PointerType*>(type)) {
            m_temp = m_builder.build_cmp_sle(lhs, rhs);
        } else if (type->is_unsigned_integer()) {
            m_temp = m_builder.build_cmp_ule(lhs, rhs);
        } else if (type->is_floating_point()) {
            m_temp = m_builder.build_cmp_fle(lhs, rhs);
        } else {
            break;
        }

        return;

    case BinaryOp::Gt:
        if (type->is_signed_integer() || dynamic_cast<const PointerType*>(type)) {
            m_temp = m_builder.build_cmp_sgt(lhs, rhs);
        } else if (type->is_unsigned_integer()) {
            m_temp = m_builder.build_cmp_ugt(lhs, rhs);
        } else if (type->is_floating_point()) {
            m_temp = m_builder.build_cmp_fgt(lhs, rhs);
        } else {
            break;
        }

        return;

    case BinaryOp::GtEq:
        if (type->is_signed_integer() ||dynamic_cast<const PointerType*>(type)) {
            m_temp = m_builder.build_cmp_sge(lhs, rhs);
        } else if (type->is_unsigned_integer()) {
            m_temp = m_builder.build_cmp_uge(lhs, rhs);
        } else if (type->is_floating_point()) {
            m_temp = m_builder.build_cmp_fge(lhs, rhs);
        } else {
            break;
        }

        return;

    default:
        break;
    }

    assert(false && "invalid cmp operator!");
}

void Codegen::codegen_logical_and(BinaryOp& node) {
    assert(m_vc == Valued);
    
    lir::BasicBlock* right_bb = lir::BasicBlock::create();
    lir::BasicBlock* merge_bb = lir::BasicBlock::create();

    node.lhs()->accept(*this);
    assert(m_temp);

    lir::Value* lhs = inject_comparison(m_temp);

    lir::BasicBlock* false_bb = m_builder.get_insert();
    m_builder.build_brif(inject_comparison(lhs), right_bb, merge_bb);

    m_func->append(right_bb);
    m_builder.set_insert(right_bb);

    m_vc = Valued;
    node.rhs()->accept(*this);
    assert(m_temp);
    
    lir::Value* rhs = inject_comparison(m_temp);

    m_builder.build_jump(merge_bb);

    lir::BasicBlock* otherwise_bb = m_builder.get_insert();
    m_func->append(merge_bb);
    m_builder.set_insert(merge_bb);

    lir::Phi* phi = m_builder.build_phi(fetch(node.type()));
    phi->add_edge(lir::Integer::get_false(m_graph), false_bb);
    phi->add_edge(rhs, otherwise_bb);

    m_temp = phi;
}

void Codegen::codegen_logical_or(BinaryOp& node) {
    assert(m_vc == Valued);
    
    lir::BasicBlock* right_bb = lir::BasicBlock::create();
    lir::BasicBlock* merge_bb = lir::BasicBlock::create();

    node.lhs()->accept(*this);
    assert(m_temp);

    lir::Value* lhs = inject_comparison(m_temp);

    lir::BasicBlock* true_bb = m_builder.get_insert();
    m_builder.build_brif(lhs, merge_bb, right_bb);

    m_func->append(right_bb);
    m_builder.set_insert(right_bb);

    m_vc = Valued;
    node.rhs()->accept(*this);
    assert(m_temp);
    lir::Value* rhs = inject_comparison(m_temp);

    m_builder.build_jump(merge_bb);

    lir::BasicBlock* otherwise_bb = m_builder.get_insert();
    m_func->append(merge_bb);
    m_builder.set_insert(merge_bb);

    lir::Phi* phi = m_builder.build_phi(fetch(node.type()));
    phi->add_edge(lir::Integer::get_true(m_graph), true_bb);
    phi->add_edge(rhs, otherwise_bb);
    
    m_temp = phi;
}

void Codegen::codegen_negation(UnaryOp& node) {
    assert(m_vc == Valued);

    node.expr()->accept(*this);
    assert(m_temp);

    if (node.type()->is_integer()) {
        if (lir::Integer* integer = dynamic_cast<lir::Integer*>(m_temp)) {
            m_temp = lir::Integer::get(m_graph, m_temp->get_type(), -integer->get_value());
        } else {
            m_temp = m_builder.build_ineg(m_temp);
        }
    } else if (node.type()->is_floating_point()) {
        if (lir::Float* fp = dynamic_cast<lir::Float*>(m_temp)) {
            m_temp = lir::Float::get(m_graph, m_temp->get_type(), -fp->get_value());
        } else {
            m_temp =  m_builder.build_fneg(m_temp);
        }
    } else {
        assert(false && "invalid negate operation!");
    }
}

void Codegen::codegen_bitwise_not(UnaryOp& node) {
    assert(m_vc == Valued);

    node.expr()->accept(*this);
    assert(m_temp);

    if (node.type()->is_integer()) {
        if (lir::Integer* integer = dynamic_cast<lir::Integer*>(m_temp)) {
            m_temp = lir::Integer::get(m_graph, m_temp->get_type(), ~integer->get_value());
        } else {
            m_temp = m_builder.build_not(m_temp);
        }
    } else {
        assert(false && "invalid bitwise not operation!");
    }
}

void Codegen::codegen_logical_not(UnaryOp& node) {
    assert(m_vc == Valued);

    node.expr()->accept(*this);
    assert(m_temp);

    lir::Type* type = m_temp->get_type();
    if (type->is_integer_type()) {
        if (lir::Integer* integer = dynamic_cast<lir::Integer*>(m_temp)) {
            m_temp = lir::Integer::get(
                m_graph, 
                lir::IntegerType::get(m_graph, 8), 
                !integer->get_value()
            );
            
            return;
        }

        m_temp = m_builder.build_cmp_ieq(m_temp, lir::Integer::get_zero(m_graph, type));
    } else if (type->is_float_type()) {
        if (lir::Float* fp = dynamic_cast<lir::Float*>(m_temp)) {
            m_temp = lir::Integer::get(
                m_graph, 
                lir::IntegerType::get(m_graph, 8), 
                !fp->get_value()
            );

            return;
        }

        m_temp = m_builder.build_cmp_feq(m_temp, lir::Float::get_zero(m_graph, type));
    } else if (type->is_pointer_type()) {
        if (dynamic_cast<lir::Null*>(m_temp)) {
            m_temp = lir::Integer::get_true(m_graph);
            return;
        }

        m_temp = m_builder.build_cmp_ieq(m_temp, lir::Null::get(m_graph, type));
    } else {
        assert(false && "invalid logical not operation!");
    }
}

void Codegen::codegen_address_of(UnaryOp& node) {
    m_vc = Addressed;
    node.expr()->accept(*this);
    assert(m_temp);
}

void Codegen::codegen_dereference(UnaryOp& node) {
    ValueContext vc = m_vc;
    
    m_vc = Valued;
    node.expr()->accept(*this);
    assert(m_temp);

    if (vc == Valued)
        m_temp = m_builder.build_load(fetch(node.type()), m_temp);
}
