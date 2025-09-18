#include "core/logger.hpp"
#include "siir/basicblock.hpp"
#include "siir/inlineasm.hpp"
#include "tree/type.hpp"
#include "siir/constant.hpp"
#include "siir/function.hpp"
#include "siir/instruction.hpp"
#include "siir/local.hpp"
#include "siir/target.hpp"
#include "siir/type.hpp"
#include "tree/decl.hpp"
#include "tree/expr.hpp"
#include "tree/root.hpp"
#include "tree/rune.hpp"
#include "tree/stmt.hpp"
#include "tree/visitor.hpp"

#include <llvm/IR/GlobalVariable.h>
#include <string>

using namespace stm;

Codegen::Codegen(Options& opts, Root& root, siir::CFG& cfg)
    : m_opts(opts), m_root(root), m_cfg(cfg), m_builder(cfg) {}

const std::string& Codegen::mangle(const Decl* decl) {
    auto it = m_mangled.find(decl);
    if (it != m_mangled.end())
        return it->second;

    return decl->get_name();
}

siir::Function* Codegen::fetch_runtime_fn(const std::string& name,
                                          const std::vector<const siir::Type*>& params,
                                          const siir::Type* ret) {
    siir::Function* fn = nullptr;
    fn = m_cfg.get_function(name);
    if (fn)
        return fn;

    const siir::FunctionType* type =
        siir::FunctionType::get(m_cfg, params, ret);

    return new siir::Function(
        m_cfg, siir::Function::LINKAGE_EXTERNAL, type, name, {});                       
}

const siir::Type* Codegen::lower_type(const Type* type) {
    if (type->is_deferred()) {
        return lower_type(type->as_deferred()->get_resolved());
    } else if (type->is_pointer()) {
        return siir::PointerType::get(m_cfg, 
            lower_type(type->as_pointer()->get_pointee()));
    } else if (type->is_struct()) {
        return siir::StructType::get(m_cfg, type->as_struct()->to_string());
    } else if (type->is_enum()) {
        return lower_type(type->as_enum()->get_underlying());
    } else if (auto blt = dynamic_cast<const BuiltinType*>(type)) {
        switch (blt->kind()) {
        case BuiltinType::Kind::Void:
            return nullptr;
        case BuiltinType::Kind::Bool:
        case BuiltinType::Kind::Char:
        case BuiltinType::Kind::SInt8:
        case BuiltinType::Kind::UInt8:
            return siir::Type::get_i8_type(m_cfg);
        case BuiltinType::Kind::SInt16:
        case BuiltinType::Kind::UInt16:
            return siir::Type::get_i16_type(m_cfg);
        case BuiltinType::Kind::SInt32:
        case BuiltinType::Kind::UInt32:
            return siir::Type::get_i32_type(m_cfg);
        case BuiltinType::Kind::SInt64:
        case BuiltinType::Kind::UInt64:
            return siir::Type::get_i64_type(m_cfg);
        case BuiltinType::Kind::Float32:
            return siir::Type::get_f32_type(m_cfg);
        case BuiltinType::Kind::Float64:
            return siir::Type::get_f64_type(m_cfg);
        }
    }

    assert(false);
}

siir::Value* Codegen::inject_bool_cmp(siir::Value* value) {
    if (value->get_type()->is_integer_type(1)) {
        return value;
    } else if (value->get_type()->is_integer_type()) {
        return m_builder.build_cmp_ine(value, 
            siir::ConstantInt::get(m_cfg, value->get_type(), 0));   
    } else if (value->get_type()->is_floating_point_type()) {
        return m_builder.build_cmp_one(value, 
            siir::ConstantFP::get(m_cfg, value->get_type(), 0.f));
    } else if (value->get_type()->is_pointer_type()) {
        return m_builder.build_cmp_ine(value, 
            siir::ConstantNull::get(m_cfg, value->get_type())); 
    } else {
        assert(false && "incompatible boolean value");
    }
}

void Codegen::lower_function(const FunctionDecl& decl) {
    siir::Function::LinkageType linkage = siir::Function::LINKAGE_INTERNAL;
    if (decl.has_decorator(Rune::Public) || decl.get_name() == "main")
        linkage = siir::Function::LINKAGE_EXTERNAL;

    std::vector<const siir::Type*> arg_types;
    std::vector<siir::Argument*> args;
    args.reserve(decl.num_params());
    arg_types.reserve(decl.num_params());
    for (auto& param : decl.get_params()) {
        const siir::Type* atype = lower_type(param->get_type());
        arg_types.push_back(atype);
        siir::Argument* arg = new siir::Argument(
            atype, param->get_name(), args.size(), nullptr);
        args.push_back(arg);
    }

    auto type = siir::FunctionType::get(
        m_cfg, arg_types, lower_type(decl.get_return_type()));
    
    siir::Function* function = new siir::Function(
        m_cfg, linkage, type, mangle(&decl), args);
}

void Codegen::impl_function(const FunctionDecl& decl) {
    siir::Function* fn = m_func = m_cfg.get_function(mangle(&decl));
    assert(fn);

    if (!decl.has_body()) 
        return;

    for (u32 idx = 0, e = decl.num_params(); idx != e; ++idx) {
        const siir::Type* arg_type = fn->get_type()->get_arg(idx);
        siir::Local* local = new siir::Local(
            m_cfg, 
            arg_type, 
            m_cfg.get_target().get_type_align(arg_type), 
            decl.get_param(idx)->get_name(),
            fn);
    }

    siir::BasicBlock* entry = new siir::BasicBlock(fn);
    m_builder.set_insert(entry);

    for (u32 idx = 0, e = decl.num_params(); idx != e; ++idx) {
        siir::Argument* arg = fn->get_arg(idx);
        const ParameterDecl* param = decl.get_param(idx);

        siir::Local* arg_local = fn->get_local(arg->get_name());
        assert(arg_local);

        m_builder.build_store(arg, arg_local);
    }

    decl.pBody->accept(*this);

    if (!m_builder.get_insert()->terminates()) {
        if (!fn->get_return_type()) {
            m_builder.build_ret_void();
        } else Logger::fatal(
            "function '" + fn->get_name() + "' does not always return",
            decl.get_span()
        );
    }

    m_func = nullptr;
    m_builder.clear_insert();
}

void Codegen::lower_structure(const StructDecl& decl) {
    siir::StructType* type = siir::StructType::get(m_cfg, decl.get_name());
    assert(type && "structure shell type not created!");

    for (auto& field : decl.get_fields()) {
        const siir::Type* field_type = lower_type(field->get_type());
        assert(field_type && "could not lower structure field type to SIIR!");

        type->append_field(field_type);
    }
}

void Codegen::visit(Root& node) {
    for (auto& import : node.imports()) {
        if (auto structure = dynamic_cast<StructDecl*>(import)) {
            siir::StructType* type = siir::StructType::create(
                m_cfg, structure->get_name(), {});
        }
    }

    for (auto& decl : node.decls()) {
        if (auto structure = dynamic_cast<StructDecl*>(decl)) {
            siir::StructType* type = siir::StructType::create(
                m_cfg, structure->get_name(), {});
        }
    }

    m_phase = PH_Declare;
    for (auto& import : node.imports()) 
        import->accept(*this);

    for (auto& decl : node.decls()) 
        decl->accept(*this);

    m_phase = PH_Define;
    for (auto& decl : node.decls()) 
        decl->accept(*this);
}

void Codegen::visit(FunctionDecl& node) {
    switch (m_phase) {
    case PH_Declare:
        lower_function(node);
        break;
    case PH_Define:
        impl_function(node);
        break;
    }
}

void Codegen::visit(VariableDecl& node) {
    const siir::Type* type = lower_type(node.get_type());
    assert(type && "could not resolve type for variable!");

    if (node.is_global()) {
        if (m_phase == PH_Declare) {
            siir::Global::LinkageType linkage = siir::Global::LINKAGE_INTERNAL;
            if (node.has_decorator(Rune::Public))
                linkage = siir::Global::LINKAGE_EXTERNAL;

            siir::Global* G = new siir::Global(
                m_cfg, type, linkage, false, mangle(&node), nullptr);
        } else if (m_phase == PH_Define) {
            siir::Global* G = m_cfg.get_global(mangle(&node));
            assert(G && "global has not been lowered correctly!");

            m_vctx = RValue;
            node.get_init()->accept(*this);
            assert(m_tmp && "global initializer does not produce a value!");
            siir::Constant* init = dynamic_cast<siir::Constant*>(m_tmp);
            assert(init && "global initializer is not a constant value!");

            G->set_initializer(init);
        }
    } else {
        siir::Local* local = new siir::Local(
            m_cfg,
            type, 
            m_cfg.get_target().get_type_align(type),
            node.get_name(), 
            m_func);
            
        if (node.has_init()) {
            m_vctx = RValue;
            node.get_init()->accept(*this);
            assert(m_tmp);

            m_builder.build_store(m_tmp, local);
        }
    }
}

void Codegen::visit(StructDecl& node) {
    switch (m_phase) {
    case PH_Declare:
        lower_structure(node);
        break;
    case PH_Define:
        break;
    }
}

void Codegen::visit(AsmStmt& node) {
    std::string string = node.string();
    std::vector<std::string> constraints = {};
    std::vector<siir::Value*> values = {};
    bool side_effects = node.is_volatile();

    for (auto& output : node.outputs()) {
        std::string constraint = "";
        if (output == "=r") {
            constraint = "=*r";
        } else if (output == "=m") {
            constraint = "=*m";
        } else {
            Logger::fatal(
                "unrecognized '__asm__' output constraint: '" + output + "'", 
                node.get_span());
        }

        constraints.push_back(constraint);
    }

    for (auto& input : node.inputs()) {
        std::string constraint = "";
        if (input == "r") {
            constraint = "r";
        } else if (input == "m") {
            constraint = "m";
        } else {
            Logger::fatal(
                "unrecognized '__asm__' input constraint: '" + input + "'", 
                node.get_span());
        }

        constraints.push_back(constraint);
    }

    for (auto& clobber : node.clobbers()) {
        constraints.push_back('~' + clobber);
    }

    for (u32 idx = 0, e = node.exprs().size(); idx != e; ++idx) {
        if (idx >= node.outputs().size()) {
            m_vctx = RValue;
        } else {
            m_vctx = LValue;
        }

        node.exprs().at(idx)->accept(*this);
        assert(m_tmp && "inline assembly operand does not produce a value!");
        values.push_back(m_tmp);
    }
    
    std::vector<const siir::Type*> operand_types(values.size(), nullptr);
    for (u32 idx = 0, e = values.size(); idx != e; ++idx)
        operand_types[idx] = values[idx]->get_type();

    const siir::FunctionType* type = siir::FunctionType::get(
        m_cfg, operand_types, nullptr);

    siir::InlineAsm* iasm = new siir::InlineAsm(
        type,
        string,
        constraints,
        side_effects
    );

    m_builder.build_call(type, iasm, values);
}

void Codegen::visit(BlockStmt& node) {
    for (auto stmt : node.stmts) stmt->accept(*this);
}

void Codegen::visit(BreakStmt& node) {
    if (m_builder.get_insert()->terminates())
        return;

    assert(m_merge);
    m_builder.build_jmp(m_merge);
}

void Codegen::visit(ContinueStmt& node) {
    if (m_builder.get_insert()->terminates())
        return;

    assert(m_merge);
    m_builder.build_jmp(m_cond);
}

void Codegen::visit(DeclStmt& node) {
    node.pDecl->accept(*this);
}

void Codegen::visit(IfStmt& node) {
    m_vctx = RValue;
    node.pCond->accept(*this);
    assert(m_tmp);

    siir::BasicBlock* then_bb = new siir::BasicBlock(m_func);
    siir::BasicBlock* else_bb = nullptr;
    siir::BasicBlock* merge_bb = new siir::BasicBlock();

    if (node.has_else()) {
        else_bb = new siir::BasicBlock();
        m_builder.build_brif(inject_bool_cmp(m_tmp), then_bb, else_bb);
    } else {
        m_builder.build_brif( inject_bool_cmp(m_tmp), then_bb, merge_bb);
    }
    
    m_builder.set_insert(then_bb);
    node.pThen->accept(*this);

    if (!m_builder.get_insert()->terminates())
        m_builder.build_jmp(merge_bb);
    
    if (node.has_else()) {
        m_func->push_back(else_bb);
        m_builder.set_insert(else_bb);
        node.pElse->accept(*this);

        if (!m_builder.get_insert()->terminates())
            m_builder.build_jmp(merge_bb);
        
    }

    if (merge_bb->has_preds()) {
        m_func->push_back(merge_bb);
        m_builder.set_insert(merge_bb);
    }
}

void Codegen::visit(WhileStmt& node) {
    siir::BasicBlock* cond_bb = new siir::BasicBlock(m_func);
    siir::BasicBlock* body_bb = new siir::BasicBlock();
    siir::BasicBlock* merge_bb = new siir::BasicBlock();

    m_builder.build_jmp(cond_bb);

    m_builder.set_insert(cond_bb);
    m_vctx = RValue;
    node.pCond->accept(*this);
    assert(m_tmp);

    m_builder.build_brif(inject_bool_cmp(m_tmp), body_bb, merge_bb);

    m_func->push_back(body_bb);
    m_builder.set_insert(body_bb);

    siir::BasicBlock* prev_cond = m_cond;
    siir::BasicBlock* prev_merge = m_merge;
    m_cond = cond_bb;
    m_merge = merge_bb;

    node.pBody->accept(*this);

    if (!m_builder.get_insert()->terminates())
        m_builder.build_jmp(cond_bb);

    m_func->push_back(merge_bb);
    m_builder.set_insert(merge_bb);
    m_cond = prev_cond;
    m_merge = prev_merge;
}

void Codegen::visit(RetStmt& node) {
    if (m_builder.get_insert()->terminates())
        return;

    if (!node.has_expr()) {
        m_builder.build_ret_void();
        return;
    }

    m_vctx = RValue;
    node.pExpr->accept(*this);
    assert(m_tmp);
    m_builder.build_ret(m_tmp);
    m_tmp = nullptr;
}

void Codegen::codegen_rune_abort(const RuneStmt& node) {
    siir::Function* abort_fn = fetch_runtime_fn("__abort");
    m_builder.build_call(abort_fn->get_type(), abort_fn, {});
}

void Codegen::codegen_rune_assert(const RuneStmt& node) {
    if (node.rune()->num_args() != 1) {
        Logger::fatal(
            "'$assert' rune must have exactly one argument",
            node.get_span());
    }

    m_vctx = RValue;
    node.rune()->args().front()->accept(*this);
    assert(m_tmp && "assert rune expression does not produce a value!");
    m_tmp = inject_bool_cmp(m_tmp);

    siir::BasicBlock* fail = new siir::BasicBlock(m_func);
    siir::BasicBlock* okay = new siir::BasicBlock(m_func);

    m_builder.build_brif(m_tmp, okay, fail);

    siir::Function* panic_fn = fetch_runtime_fn("__panic", {
        siir::PointerType::get(m_cfg, siir::Type::get_i8_type(m_cfg)),
        siir::Type::get_i64_type(m_cfg)
    });

    const SourceLocation& loc = node.rune()->args().front()->get_span().begin;
    std::string msg = loc.file.filename() + ':' + std::to_string(loc.line) + 
        ':' + std::to_string(loc.column) + ": assertion failed\n";
        
    m_builder.set_insert(fail);
    siir::Instruction* string = m_builder.build_string(
        siir::ConstantString::get(m_cfg, msg));
    m_builder.build_call(
        panic_fn->get_type(), 
        panic_fn, 
        { 
            string, 
            siir::ConstantInt::get(
                m_cfg, siir::Type::get_i64_type(m_cfg), msg.size()) 
        }
    );
    m_builder.build_unreachable();
    m_builder.set_insert(okay);
}

void Codegen::codegen_rune_write(const RuneStmt& node) {
    const Rune* rune = node.rune();
    bool is_print = rune->kind() == Rune::Print || 
        rune->kind() == Rune::Println;

    // Check that there are atleast 1-2 arguments to the rune.
    u32 min_args = is_print ? 1 : 2;
    if (rune->num_args() < min_args) {
        Logger::fatal(
            "expected atleast " + std::to_string(min_args) + " to '$" + 
                Rune::to_string(rune->kind()) + "' rune, got " + 
                std::to_string(rune->num_args()),
            node.get_span());
    }

    u32 string_idx = is_print ? 0 : 1;
    StringLiteral* strlit = dynamic_cast<StringLiteral*>(rune->args().at(string_idx));
    if (!strlit) {
        Logger::fatal(
            "expected first argument to '$print' to be a string literal",
            node.get_span());
    }
    
    // Get the stdout file descriptor as a constant integer.
    siir::Value* fd = nullptr;
    if (is_print) {
        fd = siir::ConstantInt::get(
            m_cfg, siir::Type::get_i64_type(m_cfg), 1);
    } else {

        // Lower the first argument as a "file" instance.
        Expr* file_expr = node.rune()->args().front();
        const Type* file_type = file_expr->get_type();
        if (file_type->is_deferred())
            file_type = file_type->as_deferred()->get_resolved();

        /// TODO: Adjust with mutability changes.
        //if (!file_type->is_mut() || !file_type->is_struct()) {
        bool file_type_correct = true;
        if (!file_type->is_struct()) {
            file_type_correct = false;
        } else {
            const StructDecl* decl = file_type->as_struct()->get_decl();
            if (decl->get_name() != "File" || !decl->has_decorator(Rune::Intrinsic))
                file_type_correct = false;
        }

        if (!file_type_correct) {
            Logger::fatal(
                "expected intrinsic, mutable 'File' type, got '" + 
                    file_type->to_string() + "'", 
                file_expr->get_span());
        }

        m_vctx = LValue;
        file_expr->accept(*this);
        assert(m_tmp && "$write file does not produce a value!");

        m_tmp = m_builder.build_ap(
            siir::PointerType::get(m_cfg, siir::Type::get_i64_type(m_cfg)), 
            m_tmp, 
            siir::ConstantInt::get_zero(m_cfg, siir::Type::get_i64_type(m_cfg)));
        fd = m_builder.build_load(siir::Type::get_i64_type(m_cfg), m_tmp); 
    }

    /// Get constant 10 for base 10 integer prints.
    siir::Constant* ten = siir::ConstantInt::get(
        m_cfg, siir::Type::get_i64_type(m_cfg), 10);

    std::string str = strlit->get_value();
    std::vector<std::string> parts;
    parts.reserve(rune->num_args() + 1);
    std::size_t pos = 0;
    while (1) {
        std::size_t open = str.find('{', pos);
        if (open == std::string::npos) {
            parts.push_back(str.substr(pos));
            break;
        }

        // Check if there is a proper '{}' placeholder
        if (open + 1 < str.size() && str[open + 1] == '}') {
            parts.push_back(str.substr(pos, open - pos));
            pos = open + 2; // skip "{}" in the format string.
        } else {
            // This is a lone '{' - treat it as any other character
            std::size_t next_pos = open + 1;
            std::size_t next_open = str.find('{', next_pos);
            
            // If there are no more placeholders, append everything
            if (next_open == std::string::npos) {
                parts.push_back(str.substr(pos));
                break;
            }
            
            // Check if the next '{' forms a valid placeholder
            if (next_open + 1 < str.size() && str[next_open + 1] == '}') {
                parts.push_back(str.substr(pos, next_open - pos));
                pos = next_open + 2;
            } else {
                pos = next_pos;
            }
        }
    }

    u32 num_args = is_print ? rune->num_args() - 1 : rune->num_args() - 2;
    if (parts.size() - 1 != num_args) {
        Logger::fatal(
            "argument count mismatch with bracket count, found " + 
                std::to_string(parts.size() - 1) + " bracket(s), but got " + 
                std::to_string(num_args) + " arguments", 
            node.get_span());
    }

    siir::Function* rt_print = fetch_runtime_fn(
        "__print_fd", 
        {
            siir::Type::get_i64_type(m_cfg),
            siir::PointerType::get(m_cfg, siir::Type::get_i8_type(m_cfg))
        }
    );

    for (u32 idx = 0, e = parts.size(); idx != e; ++idx) {
        std::string part = parts.at(idx);

        if (!part.empty()) {
            // This part of the string isn't empty, so we treat it like a
            // string literal.
            siir::Instruction* string = m_builder.build_string(
                siir::ConstantString::get(m_cfg, part));

            m_builder.build_call(rt_print->get_type(), rt_print, { fd, string });
        }

        if (idx >= is_print ? num_args - 1 : num_args - 2)
            continue;
    
        Expr* arg = rune->args().at(idx + (is_print ? 0 : 1));
        m_vctx = RValue;
        arg->accept(*this);
        assert(m_tmp && "print argument does not produceConstantString *string a value!");

        if (arg->get_type()->is_bool()) {
            siir::Function* rt_print_bool = fetch_runtime_fn(
                "__print_bool", 
                {
                    siir::Type::get_i64_type(m_cfg),
                    siir::Type::get_i8_type(m_cfg),
                }
            );

            m_builder.build_call(
                rt_print_bool->get_type(), rt_print_bool, { fd, m_tmp });
        } else if (arg->get_type()->is_char()) {
            siir::Function* rt_print_char = fetch_runtime_fn(
                "__print_char", 
                {
                    siir::Type::get_i64_type(m_cfg),
                    siir::Type::get_i8_type(m_cfg),
                }
            );

            m_builder.build_call(
                rt_print_char->get_type(), rt_print_char, { fd, m_tmp });
        } else if (arg->get_type()->is_signed_int()) {
            if (!m_tmp->get_type()->is_integer_type(64))
                m_tmp = m_builder.build_sext(siir::Type::get_i64_type(m_cfg), m_tmp);

            siir::Function* rt_print_si = fetch_runtime_fn(
                "__print_si", 
                {
                    siir::Type::get_i64_type(m_cfg),
                    siir::Type::get_i64_type(m_cfg),
                    siir::Type::get_i64_type(m_cfg),
                }
            );

            m_builder.build_call(
                rt_print_si->get_type(), rt_print_si, { fd, m_tmp, ten });
        } else if (arg->get_type()->is_unsigned_int()) {
            if (!m_tmp->get_type()->is_integer_type(64))
                m_tmp = m_builder.build_zext(siir::Type::get_i64_type(m_cfg), m_tmp);

            siir::Function* rt_print_ui = fetch_runtime_fn(
                "__print_ui", 
                {
                    siir::Type::get_i64_type(m_cfg),
                    siir::Type::get_i64_type(m_cfg),
                    siir::Type::get_i64_type(m_cfg),
                }
            );

            m_builder.build_call(
                rt_print_ui->get_type(), rt_print_ui, { fd, m_tmp, ten });
        } else if (m_tmp->get_type()->is_floating_point_type(32)) {
            siir::Function* rt_print_float = fetch_runtime_fn(
                "__print_float", 
                {
                    siir::Type::get_i64_type(m_cfg),
                    siir::Type::get_f32_type(m_cfg),
                }
            );

            m_builder.build_call(
                rt_print_float->get_type(), rt_print_float, { fd, m_tmp });
        } else if (m_tmp->get_type()->is_floating_point_type(64)) {
            siir::Function* rt_print_double = fetch_runtime_fn(
                "__print_double", 
                {
                    siir::Type::get_i64_type(m_cfg),
                    siir::Type::get_f64_type(m_cfg),
                }
            );

            m_builder.build_call(
                rt_print_double->get_type(), rt_print_double, { fd, m_tmp });
        } else if (arg->get_type()->is_pointer()) {
            siir::Function* rt_print_ptr = fetch_runtime_fn(
                "__print_ptr", 
                {
                    siir::Type::get_i64_type(m_cfg),
                    siir::PointerType::get(m_cfg, nullptr),
                }
            );

            m_builder.build_call(
                rt_print_ptr->get_type(), rt_print_ptr, { fd, m_tmp });
        } else {
            Logger::fatal(
                "unsupported operand type to '$print': '" + 
                    arg->get_type()->to_string() + "'", 
                arg->get_span());
        }
    }

    if (rune->kind() == Rune::Println || rune->kind() == Rune::Writeln) {
        siir::Instruction* string = m_builder.build_string(
            siir::ConstantString::get(m_cfg, "\n"));
        m_builder.build_call(rt_print->get_type(), rt_print, { fd, string });
    }
}

void Codegen::visit(RuneStmt& node) {
    switch (node.rune()->kind()) {
    case Rune::Abort:
        codegen_rune_abort(node);
        break;
    case Rune::Asm:
        break;
    case Rune::Assert:
        codegen_rune_assert(node);
        break;
    case Rune::If:
        break;
    case Rune::Print:
    case Rune::Println:
    case Rune::Write:
    case Rune::Writeln:
        codegen_rune_write(node);
        break;
    default:
        assert(false && 
            "cannot generate code for a non-statement rune as a statement!");
    }
}

void Codegen::visit(BoolLiteral& node) {
    m_tmp = siir::ConstantInt::get(m_cfg, siir::Type::get_i1_type(m_cfg), 
        node.get_value());
}

void Codegen::visit(IntegerLiteral& node) {
    m_tmp = siir::ConstantInt::get(m_cfg, lower_type(node.get_type()), 
        node.get_value());
}

void Codegen::visit(FloatLiteral& node) {
    m_tmp = siir::ConstantFP::get(m_cfg, lower_type(node.get_type()), 
        node.get_value());
}

void Codegen::visit(CharLiteral& node) {
    m_tmp = siir::ConstantInt::get(m_cfg, siir::Type::get_i8_type(m_cfg), 
        node.get_value());
}

void Codegen::visit(StringLiteral& node) {
    m_tmp = m_builder.build_string(
        siir::ConstantString::get(m_cfg, node.get_value()));
}

void Codegen::visit(NullLiteral& node) {
    m_tmp = siir::ConstantNull::get(m_cfg, lower_type(node.get_type()));
}

void Codegen::visit(BinaryExpr& node) {
    switch (node.get_operator()) {
    case BinaryExpr::Operator::Assign:
        return codegen_binary_assign(node);
    case BinaryExpr::Operator::Add:
        return codegen_binary_add(node);
    case BinaryExpr::Operator::Add_Assign:
        return codegen_binary_add_assign(node);
    case BinaryExpr::Operator::Sub:
        return codegen_binary_sub(node);
    case BinaryExpr::Operator::Sub_Assign:
        return codegen_binary_sub_assign(node);
    case BinaryExpr::Operator::Mul:
        return codegen_binary_mul(node);
    case BinaryExpr::Operator::Mul_Assign:
        return codegen_binary_mul_assign(node);
    case BinaryExpr::Operator::Div:
        return codegen_binary_div(node);
    case BinaryExpr::Operator::Div_Assign:
        return codegen_binary_div_assign(node);
    case BinaryExpr::Operator::Mod:
        return codegen_binary_mod(node);
    case BinaryExpr::Operator::Mod_Assign:
        return codegen_binary_mod_assign(node);
    case BinaryExpr::Operator::Equals:
        return codegen_binary_eq(node);
    case BinaryExpr::Operator::Not_Equals:
        return codegen_binary_ne(node);
    case BinaryExpr::Operator::Less_Than:
        return codegen_binary_lt(node);
    case BinaryExpr::Operator::Less_Than_Equals:
        return codegen_binary_lte(node);
    case BinaryExpr::Operator::Greater_Than:
        return codegen_binary_gt(node);
    case BinaryExpr::Operator::Greater_Than_Equals:
        return codegen_binary_gte(node);
    case BinaryExpr::Operator::Bitwise_And:
        return codegen_binary_bitwise_and(node);
    case BinaryExpr::Operator::Bitwise_And_Assign:
        return codegen_binary_bitwise_and_assign(node);
    case BinaryExpr::Operator::Bitwise_Or:
        return codegen_binary_bitwise_or(node);
    case BinaryExpr::Operator::Bitwise_Or_Assign:
        return codegen_binary_bitwise_or_assign(node);
    case BinaryExpr::Operator::Bitwise_Xor:
        return codegen_binary_bitwise_xor(node);
    case BinaryExpr::Operator::Bitwise_Xor_Assign:
        return codegen_binary_bitwise_xor_assign(node);
    case BinaryExpr::Operator::Logical_And:
        return codegen_binary_logical_and(node);
    case BinaryExpr::Operator::Logical_Or:
        return codegen_binary_logical_or(node);
    case BinaryExpr::Operator::Left_Shift:
        return codegen_binary_left_shift(node);
    case BinaryExpr::Operator::Left_Shift_Assign:
        return codegen_binary_left_shift_assign(node);
    case BinaryExpr::Operator::Right_Shift:
        return codegen_binary_right_shift(node);
    case BinaryExpr::Operator::Right_Shift_Assign:
        return codegen_binary_right_shift_assign(node);
    default:
        assert(false && "operator not implemented");
    }
}

void Codegen::codegen_binary_assign(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rval = m_tmp;
    
    m_vctx = LValue;
    node.pLeft->accept(*this);
    assert(m_tmp);

    m_builder.build_store(rval, m_tmp);
}

void Codegen::codegen_binary_add(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_pointer_type() 
      && rhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_ap(
            lower_type(node.get_type()), 
            lhs, 
            rhs);
    } else if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_iadd(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_fadd(lhs, rhs);
    } else Logger::fatal(
        "unsupported '+' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_add_assign(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_pointer_type() 
      && rhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_ap(
            lower_type(node.get_type()), 
            lhs, 
            rhs);
    } else if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_iadd(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_fadd(lhs, rhs);
    } else Logger::fatal(
        "unsupported '+' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );

    siir::Value* value = m_tmp;
    m_vctx = LValue;
    node.pLeft->accept(*this);
    assert(m_tmp && "binary lhs does not produce an lvalue!");

    m_builder.build_store(value, m_tmp);
    m_tmp = value;
}

void Codegen::codegen_binary_sub(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_pointer_type() 
      && rhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_ap(
            lower_type(node.get_type()), 
            lhs, 
            m_builder.build_ineg(rhs));
    } else if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_isub(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_fsub(lhs, rhs);
    } else Logger::fatal(
        "unsupported '+' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_sub_assign(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_pointer_type() 
      && rhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_ap(
            lower_type(node.get_type()), 
            lhs, 
            m_builder.build_ineg(rhs));
    } else if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_isub(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_fsub(lhs, rhs);
    } else Logger::fatal(
        "unsupported '+' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );

    siir::Value* value = m_tmp;
    m_vctx = LValue;
    node.pLeft->accept(*this);
    assert(m_tmp && "binary lhs does not produce an lvalue!");

    m_builder.build_store(value, m_tmp);
    m_tmp = value;
}

void Codegen::codegen_binary_mul(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (node.get_lhs()->get_type()->is_signed_int()) {
        m_tmp = m_builder.build_smul(lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_umul(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_fmul(lhs, rhs);
    } else Logger::fatal(
        "unsupported '*' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_mul_assign(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (node.get_lhs()->get_type()->is_signed_int()) {
        m_tmp = m_builder.build_smul(lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_umul(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_fmul(lhs, rhs);
    } else Logger::fatal(
        "unsupported '*' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );

    siir::Value* value = m_tmp;
    m_vctx = LValue;
    node.pLeft->accept(*this);
    assert(m_tmp && "binary lhs does not produce an lvalue!");

    m_builder.build_store(value, m_tmp);
    m_tmp = value;
}

void Codegen::codegen_binary_div(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (node.get_lhs()->get_type()->is_signed_int()) {
        m_tmp = m_builder.build_sdiv(lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_udiv(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_fdiv(lhs, rhs);
    } else Logger::fatal(
        "unsupported '/' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    ); 
}

void Codegen::codegen_binary_div_assign(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (node.get_lhs()->get_type()->is_signed_int()) {
        m_tmp = m_builder.build_sdiv(lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_udiv(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_fdiv(lhs, rhs);
    } else Logger::fatal(
        "unsupported '/' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    ); 

    siir::Value* value = m_tmp;
    m_vctx = LValue;
    node.pLeft->accept(*this);
    assert(m_tmp && "binary lhs does not produce an lvalue!");

    m_builder.build_store(value, m_tmp);
    m_tmp = value;
}

void Codegen::codegen_binary_mod(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (node.get_lhs()->get_type()->is_signed_int()) {
        m_tmp = m_builder.build_srem(lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_urem(lhs, rhs);
    } else Logger::fatal(
        "unsupported '/' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    ); 
}

void Codegen::codegen_binary_mod_assign(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (node.get_lhs()->get_type()->is_signed_int()) {
        m_tmp = m_builder.build_srem(lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_urem(lhs, rhs);
    } else Logger::fatal(
        "unsupported '/' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    ); 
    
    siir::Value* value = m_tmp;
    m_vctx = LValue;
    node.pLeft->accept(*this);
    assert(m_tmp && "binary lhs does not produce an lvalue!");

    m_builder.build_store(value, m_tmp);
    m_tmp = value;
}

void Codegen::codegen_binary_eq(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_integer_type() 
      || lhs->get_type()->is_pointer_type()) {
        m_tmp = m_builder.build_cmp_ieq(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp_oeq(lhs, rhs);
    } else Logger::fatal(
        "unsupported '==' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );  
}

void Codegen::codegen_binary_ne(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_integer_type() 
      || lhs->get_type()->is_pointer_type()) {
        m_tmp = m_builder.build_cmp_ine(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp_one(lhs, rhs);
    } else Logger::fatal(
        "unsupported '!=' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_lt(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (node.get_lhs()->get_type()->is_signed_int()
      || node.get_lhs()->get_type()->is_pointer()) {
        m_tmp = m_builder.build_cmp_slt(lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_cmp_ult(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp_olt(lhs, rhs);
    } else Logger::fatal(
        "unsupported '<' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_lte(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (node.get_lhs()->get_type()->is_signed_int()
      || node.get_lhs()->get_type()->is_pointer()) {
        m_tmp = m_builder.build_cmp_sle(lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_cmp_ule(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp_ole(lhs, rhs);
    } else Logger::fatal(
        "unsupported '<=' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_gt(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (node.get_lhs()->get_type()->is_signed_int()
      || node.get_lhs()->get_type()->is_pointer()) {
        m_tmp = m_builder.build_cmp_sgt(lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_cmp_ugt(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp_ogt(lhs, rhs);
    } else Logger::fatal(
        "unsupported '>' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_gte(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (node.get_lhs()->get_type()->is_signed_int()
      || node.get_lhs()->get_type()->is_pointer()) {
        m_tmp = m_builder.build_cmp_sge(lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_cmp_uge(lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp_oge(lhs, rhs);
    } else Logger::fatal(
        "unsupported '>=' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_bitwise_and(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_and(lhs, rhs);
    } else Logger::fatal(
        "unsupported '&' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_bitwise_and_assign(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_and(lhs, rhs);
    } else Logger::fatal(
        "unsupported '&' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );

    siir::Value* value = m_tmp;
    m_vctx = LValue;
    node.pLeft->accept(*this);
    assert(m_tmp && "binary lhs does not produce an lvalue!");

    m_builder.build_store(value, m_tmp);
    m_tmp = value;
}

void Codegen::codegen_binary_bitwise_or(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_or(lhs, rhs);
    } else Logger::fatal(
        "unsupported '|' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_bitwise_or_assign(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_or(lhs, rhs);
    } else Logger::fatal(
        "unsupported '|' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
    
    siir::Value* value = m_tmp;
    m_vctx = LValue;
    node.pLeft->accept(*this);
    assert(m_tmp && "binary lhs does not produce an lvalue!");

    m_builder.build_store(value, m_tmp);
    m_tmp = value;
}

void Codegen::codegen_binary_bitwise_xor(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_xor(lhs, rhs);
    } else Logger::fatal(
        "unsupported '^' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_bitwise_xor_assign(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_xor(lhs, rhs);
    } else Logger::fatal(
        "unsupported '^' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );

    siir::Value* value = m_tmp;
    m_vctx = LValue;
    node.pLeft->accept(*this);
    assert(m_tmp && "binary lhs does not produce an lvalue!");

    m_builder.build_store(value, m_tmp);
    m_tmp = value;
}

void Codegen::codegen_binary_logical_and(const BinaryExpr& node) {
    siir::BasicBlock* right_bb = new siir::BasicBlock();
    siir::BasicBlock* merge_bb = new siir::BasicBlock();

    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp && "binary lhs does not produce a value!");
    siir::Value* lhs = inject_bool_cmp(m_tmp);

    siir::BasicBlock* false_bb = m_builder.get_insert();
    m_builder.build_brif(lhs, right_bb, merge_bb);

    m_func->push_back(right_bb);
    m_builder.set_insert(right_bb);

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp && "binary rhs does not produce a value!");
    siir::Value *rhs = inject_bool_cmp(m_tmp);
    
    m_builder.build_jmp(merge_bb);

    siir::BasicBlock* otherwise = m_builder.get_insert();
    m_func->push_back(merge_bb);
    m_builder.set_insert(merge_bb);
    siir::Instruction* phi = m_builder.build_phi(siir::Type::get_i1_type(m_cfg));
    phi->add_incoming(m_cfg, siir::ConstantInt::get_false(m_cfg), false_bb);
    phi->add_incoming(m_cfg, rhs, otherwise);
    
    m_tmp = phi;
}

void Codegen::codegen_binary_logical_or(const BinaryExpr& node) {
    siir::BasicBlock* right_bb = new siir::BasicBlock();
    siir::BasicBlock* merge_bb = new siir::BasicBlock();

    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp && "binary lhs does not produce a value!");
    siir::Value* lhs = inject_bool_cmp(m_tmp);

    siir::BasicBlock* true_bb = m_builder.get_insert();
    m_builder.build_brif(lhs, merge_bb, right_bb);

    m_func->push_back(right_bb);
    m_builder.set_insert(right_bb);

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp && "binary rhs does not produce a value!");
    siir::Value *rhs = inject_bool_cmp(m_tmp);
    
    m_builder.build_jmp(merge_bb);

    siir::BasicBlock* otherwise = m_builder.get_insert();
    m_func->push_back(merge_bb);
    m_builder.set_insert(merge_bb);
    siir::Instruction* phi = m_builder.build_phi(siir::Type::get_i1_type(m_cfg));
    phi->add_incoming(m_cfg, siir::ConstantInt::get_true(m_cfg), true_bb);
    phi->add_incoming(m_cfg, rhs, otherwise);
    
    m_tmp = phi;
}

void Codegen::codegen_binary_left_shift(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_shl(lhs, rhs);
    } else Logger::fatal(
        "unsupported '<<' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_left_shift_assign(const BinaryExpr& node) {
    
}

void Codegen::codegen_binary_right_shift(const BinaryExpr& node) {
    m_vctx = RValue;
    node.pLeft->accept(*this);
    assert(m_tmp);
    siir::Value* lhs = m_tmp;

    m_vctx = RValue;
    node.pRight->accept(*this);
    assert(m_tmp);
    siir::Value* rhs = m_tmp;

    if (node.get_lhs()->get_type()->is_signed_int()) {
        m_tmp = m_builder.build_sar(lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_shr(lhs, rhs);
    } else Logger::fatal(
        "unsupported '>>' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_right_shift_assign(const BinaryExpr& node) {
    
}

void Codegen::visit(UnaryExpr& node) {
    switch (node.get_operator()) {
    case UnaryExpr::Operator::Increment:
        return codegen_unary_increment(node);
    case UnaryExpr::Operator::Decrement:
        return codegen_unary_decrement(node);
    case UnaryExpr::Operator::Dereference:
        return codegen_unary_dereference(node);
    case UnaryExpr::Operator::Address_Of:
        return codegen_unary_address_of(node);
    case UnaryExpr::Operator::Negate:
        return codegen_unary_negate(node);
    case UnaryExpr::Operator::Logical_Not:
        return codegen_unary_logical_not(node);
    case UnaryExpr::Operator::Bitwise_Not:
        return codegen_unary_bitwise_not(node);
    default:
        assert(false && "operator not implemented");
    }
}

void Codegen::codegen_unary_increment(const UnaryExpr& node) {
    ValueContext vctx = m_vctx;
    m_vctx = LValue;
    node.pExpr->accept(*this);
    assert(m_tmp);

    siir::Value* lvalue = m_tmp;
    siir::Value* preop = m_builder.build_load(
        lower_type(node.get_type()), lvalue);

    if (preop->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_iadd(
            preop, siir::ConstantInt::get(m_cfg, preop->get_type(), 1));
    } else if (preop->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_fadd(
            preop, siir::ConstantFP::get(m_cfg, preop->get_type(), 1.f));
    } else if (preop->get_type()->is_pointer_type()) {
        m_tmp = m_builder.build_ap(
            lower_type(node.get_type()), 
            preop, 
            siir::ConstantInt::get(m_cfg, siir::Type::get_i64_type(m_cfg), 1));
    } else Logger::fatal(
        "unsupported '++' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );

    m_builder.build_store(m_tmp, lvalue);

    if (node.is_postfix())
        m_tmp = preop;
}

void Codegen::codegen_unary_decrement(const UnaryExpr& node) {
    ValueContext vctx = m_vctx;
    m_vctx = LValue;
    node.pExpr->accept(*this);
    assert(m_tmp);

    siir::Value* lvalue = m_tmp;
    siir::Value* preop = m_builder.build_load(
        lower_type(node.get_type()), lvalue);

    if (preop->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_isub(
            preop, siir::ConstantInt::get(m_cfg, preop->get_type(), 1));
    } else if (preop->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_fsub(
            preop, siir::ConstantFP::get(m_cfg, preop->get_type(), 1.f));
    } else if (preop->get_type()->is_pointer_type()) {
        m_tmp = m_builder.build_ap(
            lower_type(node.get_type()), 
            preop, 
            siir::ConstantInt::get(m_cfg, siir::Type::get_i64_type(m_cfg), -1));
    } else Logger::fatal(
        "unsupported '--' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );

    m_builder.build_store(m_tmp, lvalue);

    if (node.is_postfix())
        m_tmp = preop;
}

void Codegen::codegen_unary_dereference(const UnaryExpr& node) {
    ValueContext vctx = m_vctx;
    m_vctx = RValue;
    node.pExpr->accept(*this);
    assert(m_tmp);

    if (vctx == RValue)
        m_tmp = m_builder.build_load(lower_type(node.get_type()), m_tmp);
}

void Codegen::codegen_unary_address_of(const UnaryExpr& node) {
    m_vctx = LValue;
    node.pExpr->accept(*this);
    assert(m_tmp);
}

void Codegen::codegen_unary_negate(const UnaryExpr& node) {
    m_vctx = RValue;
    node.pExpr->accept(*this);
    assert(m_tmp);

    if (m_tmp->get_type()->is_integer_type()
      || m_tmp->get_type()->is_pointer_type()) {
        m_tmp = m_builder.build_ineg(m_tmp);
    } else if (m_tmp->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_fneg(m_tmp);
    } else Logger::fatal(
        "unsupported '-' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_unary_logical_not(const UnaryExpr& node) {
    m_vctx = RValue;
    node.pExpr->accept(*this);
    assert(m_tmp);

    siir::Value* value = m_tmp;

    if (value->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_cmp_ieq(
            value, siir::ConstantInt::get(m_cfg, value->get_type(), 0));
    } else if (value->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp_oeq(
            value, siir::ConstantFP::get(m_cfg, value->get_type(), 1.f));
    } else if (value->get_type()->is_pointer_type()) {
        m_tmp = m_builder.build_cmp_ieq(
            value, siir::ConstantNull::get(m_cfg, value->get_type()));
    } else Logger::fatal(
        "unsupported '!' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_unary_bitwise_not(const UnaryExpr& node) {
    m_vctx = RValue;
    node.pExpr->accept(*this);
    assert(m_tmp);

    if (m_tmp->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_not(m_tmp);
    } else Logger::fatal(
        "unsupported '~' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::visit(CastExpr& node) {
    m_vctx = RValue;
    node.pExpr->accept(*this);
    assert(m_tmp);

    const siir::Type* src_type = lower_type(node.get_expr()->get_type());
    const siir::Type* dst_type = lower_type(node.get_type());
    if (*src_type == *dst_type)
        return;

    const siir::Target& target = m_cfg.get_target();
    u32 src_sz = target.get_type_size(src_type);
    u32 dst_sz = target.get_type_size(dst_type);

    siir::Type::Kind src_kind = src_type->get_kind();
    siir::Type::Kind dst_kind = dst_type->get_kind();

    if (src_type->is_integer_type() 
      && dst_type->is_integer_type()) {
        // Integer -> Integer casts.

        if (src_sz == dst_sz)
            return;

        // Fold possible constants here if possible.
        if (auto constant = dynamic_cast<siir::ConstantInt*>(m_tmp))
            m_tmp = siir::ConstantInt::get(
                m_cfg, dst_type, constant->get_value());
        else if (src_sz > dst_sz)
            m_tmp = m_builder.build_itrunc(dst_type, m_tmp);
        else if (node.get_expr()->get_type()->is_signed_int())
            m_tmp = m_builder.build_sext(dst_type, m_tmp);
        else
            m_tmp = m_builder.build_zext(dst_type, m_tmp);
    } else if (src_type->is_floating_point_type() 
      && dst_type->is_floating_point_type()) {
        // Floating point -> Floating point casts.
        if (src_sz == dst_sz)
            return;

        // Fold possible constants here if possible.
        if (auto constant = dynamic_cast<siir::ConstantFP*>(m_tmp))
            m_tmp = siir::ConstantFP::get(
                m_cfg, dst_type, constant->get_value());
        else if (src_sz > dst_sz) // Downcasting.
            m_tmp = m_builder.build_ftrunc(dst_type, m_tmp);
        else // Upcasting.
            m_tmp = m_builder.build_fext(dst_type, m_tmp);
    } else if (src_type->is_integer_type()
      && dst_type->is_floating_point_type()) {
        // Integer -> Floating point conversions.

        if (auto constant = dynamic_cast<siir::ConstantInt*>(m_tmp))
            m_tmp = siir::ConstantFP::get(m_cfg, dst_type, constant->get_value());
        else if (node.get_expr()->get_type()->is_signed_int())
            m_tmp = m_builder.build_si2fp(dst_type, m_tmp);
        else if (node.get_expr()->get_type()->is_unsigned_int())
            m_tmp = m_builder.build_ui2fp(dst_type, m_tmp);
    } else if (src_type->is_floating_point_type()
      && dst_type->is_integer_type()) {
        // Floating point -> Integer conversions.

        if (auto constant = dynamic_cast<siir::ConstantFP*>(m_tmp))
            m_tmp = siir::ConstantInt::get(m_cfg, dst_type, constant->get_value());
        else if (node.get_type()->is_signed_int())
            m_tmp = m_builder.build_fp2si(dst_type, m_tmp);
        else if (node.get_type()->is_unsigned_int())
            m_tmp = m_builder.build_fp2ui(dst_type, m_tmp);
    } else if (src_type->is_pointer_type() 
      && dst_type->is_pointer_type()) {
        // Pointer -> Pointer reinterpretations.
        
        if (dynamic_cast<siir::ConstantNull*>(m_tmp)) {
            m_tmp = siir::ConstantNull::get(m_cfg, dst_type);
        } else {
            m_tmp = m_builder.build_reint(dst_type, m_tmp);
        }
    } else if (src_type->is_array_type()
      && dst_type->is_pointer_type()) {
        // Array -> Pointer decay.
       m_tmp = m_builder.build_reint(dst_type, m_tmp);
    } else if (src_type->is_integer_type()
      && dst_type->is_pointer_type()) {
        // Integer -> Pointer casts.
        m_tmp = m_builder.build_i2p(dst_type, m_tmp);
    } else if (src_type->is_pointer_type()
      && dst_type->is_integer_type()) {
        // Pointer -> Integer casts.
        m_tmp = m_builder.build_p2i(dst_type, m_tmp);
    } else Logger::fatal(   
        "unsupported cast '" + node.get_expr()->get_type()->to_string() + 
            "' to '" + node.get_type()->to_string() + "'",
        node.get_span()
    );
}

void Codegen::visit(ParenExpr& node) {
    node.pExpr->accept(*this);
}

void Codegen::visit(SizeofExpr& node) {
    m_tmp = siir::ConstantInt::get(m_cfg, lower_type(node.get_type()), 
        m_cfg.get_target().get_type_size(lower_type(node.get_target())));
}

void Codegen::visit(SubscriptExpr& node) {
    ValueContext vctx = m_vctx;
    siir::Value* base = nullptr;
    siir::Value* idx = nullptr;
    const siir::Type* type = lower_type(node.get_type());

    m_vctx = LValue;
    if (node.get_base()->get_type()->is_pointer())
        m_vctx = RValue;

    node.pBase->accept(*this);
    assert(m_tmp);
    base = m_tmp;

    m_vctx = RValue;
    node.pIndex->accept(*this);
    assert(m_tmp);
    idx = m_tmp;
    
    m_tmp = m_builder.build_ap(
        siir::PointerType::get(m_cfg, type), base, idx);

    if (vctx == RValue)
        m_tmp = m_builder.build_load(type, m_tmp);
}

void Codegen::visit(ReferenceExpr& node) {
    if (auto value = dynamic_cast<const EnumValueDecl*>(node.get_decl())) {
        // If the referenced declaration is an enum value, then it can
        // resolved at this point to it's integer value.
        m_tmp = siir::ConstantInt::get(
            m_cfg, lower_type(value->get_type()), value->get_value());
        return;
    }

    auto var = dynamic_cast<const VariableDecl*>(node.get_decl());
    if (var && var->is_global()) {
        siir::Global* global = m_cfg.get_global(mangle(node.get_decl()));
        assert(global && "unresolved reference to global!");

        m_tmp = global;

        if (m_vctx == RValue)
            m_tmp = m_builder.build_load(lower_type(node.get_type()), global);
    } else {
        // Resolve the referenced local in the current function.
        siir::Local* local = m_func->get_local(node.get_name());
        assert(local && "unresolved reference to local!");

        m_tmp = local;

        if (m_vctx == RValue)
            m_tmp = m_builder.build_load(lower_type(node.get_type()), local);    
    }
}

void Codegen::visit(MemberExpr& node) {
    ValueContext vc = m_vctx;
    siir::Value* base = nullptr;

    const siir::Type* base_type = lower_type(node.get_base()->get_type());
    const siir::StructType* struct_type = nullptr;

    if (base_type->is_struct_type()) {
        struct_type = static_cast<const siir::StructType*>(base_type);
    } else if (struct_type->is_pointer_type()) {
        struct_type = static_cast<const siir::StructType*>(
            static_cast<const siir::PointerType*>(base_type)->get_pointee());
    }

    m_vctx = LValue;
    if (base_type->is_pointer_type())
        m_vctx = RValue;

    node.pBase->accept(*this);
    assert(m_tmp && "member access base does not produce a value!");

    u32 field_idx = static_cast<const FieldDecl*>(node.get_decl())->get_index();

    const siir::Type* field_type = lower_type(node.get_type());
    m_tmp = m_builder.build_ap(
        siir::PointerType::get(m_cfg, field_type), 
        m_tmp, 
        siir::ConstantInt::get(m_cfg, siir::Type::get_i64_type(m_cfg), field_idx));

    if (vc == RValue)
        m_tmp = m_builder.build_load(field_type, m_tmp);
}

void Codegen::visit(CallExpr& node) {
    auto target = static_cast<const FunctionDecl*>(node.get_decl());
    if (target->has_decorator(Rune::Deprecated)) {
        Logger::warn(
            "function '" + target->get_name() + "' has been marked deprecated",
            node.get_span());
    }

    siir::Function* callee = m_cfg.get_function(mangle(node.get_decl()));
    assert(callee);

    std::vector<siir::Value*> args;
    args.reserve(node.num_args());
    for (auto arg : node.args) {
        m_vctx = RValue;
        arg->accept(*this);
        assert(m_tmp);
        args.push_back(m_tmp);
    }

    m_tmp = m_builder.build_call(callee->get_type(), callee, args);
}

void Codegen::visit(RuneExpr& node) {
    switch (node.rune()->kind()) {
    case Rune::Comptime:
        m_tmp = siir::ConstantInt::get_false(m_cfg);
        break;
    case Rune::Path:
        m_tmp = m_builder.build_string(
            siir::ConstantString::get(m_cfg, m_cfg.get_file().absolute()));
        break;
    default:
        assert(false && 
            "cannot generate code for a non-value rune as an expression!");
    }
}
