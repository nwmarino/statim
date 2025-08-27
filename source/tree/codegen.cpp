#include "core/logger.hpp"
#include "core/type.hpp"
#include "siir/argument.hpp"
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

using namespace stm;

Codegen::Codegen(Options& opts, Root& root, siir::CFG& cfg)
    : m_opts(opts), m_root(root), m_cfg(cfg), m_builder(cfg) {}

const std::string& Codegen::mangle(const Decl* decl) {
    auto it = m_mangled.find(decl);
    if (it != m_mangled.end())
        return it->second;

    return decl->get_name();
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
        switch (blt->get_kind()) {
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
        return m_builder.build_cmp(siir::CmpInst::CMP_Ne, value, 
            siir::ConstantInt::get(m_cfg, value->get_type(), 0));   
    } else if (value->get_type()->is_floating_point_type()) {
        return m_builder.build_cmp(siir::CmpInst::CMP_One, value, 
            siir::ConstantFP::get(m_cfg, value->get_type(), 0.f));
    } else if (value->get_type()->is_pointer_type()) {
        return m_builder.build_cmp(siir::CmpInst::CMP_Ne, value, 
            siir::ConstantNull::get(m_cfg, value->get_type())); 
    } else {
        assert(false && "incompatible boolean value");
    }
}

void Codegen::lower_function(const FunctionDecl& decl) {
    auto linkage = siir::Function::External;

    std::vector<const siir::Type*> arg_types;
    std::vector<siir::FunctionArgument*> args;
    args.reserve(decl.num_params());
    arg_types.reserve(decl.num_params());
    for (auto& param : decl.get_params()) {
        const siir::Type* atype = lower_type(param->get_type());
        arg_types.push_back(atype);
        siir::FunctionArgument* arg = siir::FunctionArgument::create(
            args.size(), atype, param->get_name(), nullptr);
    }

    auto type = siir::FunctionType::get(
        m_cfg, arg_types, lower_type(decl.get_return_type()));
    
    auto function = siir::Function::create(
       type, linkage, args, &m_cfg, mangle(&decl));
}

void Codegen::impl_function(const FunctionDecl& decl) {
    siir::Function* fn = m_func = m_cfg.get_function(mangle(&decl));
    assert(fn);

    if (!decl.has_body()) 
        return;

    for (u32 idx = 0, e = decl.num_params(); idx != e; ++idx) {
        const siir::Type* arg_type = fn->get_type()->get_arg(idx);
        siir::Local* local = siir::Local::create(
            arg_type, 
            m_cfg.get_target().get_type_align(arg_type), 
            siir::PointerType::get(m_cfg, arg_type),
            decl.get_param(idx)->get_name(),
            fn);
    }

    siir::BasicBlock* entry = siir::BasicBlock::create({}, fn);
    m_builder.set_insert(entry);

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

}

void Codegen::visit(Root& node) {
    for (auto import : node.imports)
        if (auto structure = dynamic_cast<StructDecl*>(import))
            lower_structure(*structure);

    for (auto decl : node.decls)
        if (auto structure = dynamic_cast<StructDecl*>(decl))
            lower_structure(*structure);

    m_phase = PH_Declare;
    for (auto import : node.imports) import->accept(*this);
    for (auto decl : node.decls) decl->accept(*this);

    m_phase = PH_Define;
    for (auto decl : node.decls) decl->accept(*this);
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
    siir::Local* local = siir::Local::create(
        type, 
        m_cfg.get_target().get_type_align(type), 
        siir::PointerType::get(m_cfg, type), 
        node.get_name(), 
        m_func);
        
    if (node.has_init()) {
        m_vctx = RValue;
        node.pInit->accept(*this);
        assert(m_tmp);

        m_builder.build_store(m_tmp, local);
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

void Codegen::visit(BlockStmt& node) {
    for (auto stmt : node.stmts) stmt->accept(*this);
}

void Codegen::visit(BreakStmt& node) {
    if (m_builder.get_insert()->terminates())
        return;

    assert(m_merge);
    m_builder.build_jmp(siir::BlockAddress::get(m_cfg, m_merge));
}

void Codegen::visit(ContinueStmt& node) {
    if (m_builder.get_insert()->terminates())
        return;

    assert(m_merge);
    m_builder.build_jmp(siir::BlockAddress::get(m_cfg, m_cond));
}

void Codegen::visit(DeclStmt& node) {
    node.pDecl->accept(*this);
}

void Codegen::visit(IfStmt& node) {
    m_vctx = RValue;
    node.pCond->accept(*this);
    assert(m_tmp);

    siir::BasicBlock* then_bb = siir::BasicBlock::create({}, m_func);
    siir::BasicBlock* else_bb = nullptr;
    siir::BasicBlock* merge_bb = siir::BasicBlock::create({});

    if (node.has_else()) {
        else_bb = siir::BasicBlock::create({});
        m_builder.build_brif(
            inject_bool_cmp(m_tmp), 
            siir::BlockAddress::get(m_cfg, then_bb), 
            siir::BlockAddress::get(m_cfg, else_bb));
    } else {
        m_builder.build_brif(
            inject_bool_cmp(m_tmp), 
            siir::BlockAddress::get(m_cfg, then_bb), 
            siir::BlockAddress::get(m_cfg, merge_bb));
    }
    
    m_builder.set_insert(then_bb);
    node.pThen->accept(*this);

    if (!m_builder.get_insert()->terminates())
        m_builder.build_jmp(siir::BlockAddress::get(m_cfg, merge_bb));
    
    if (node.has_else()) {
        m_func->push_back(else_bb);
        m_builder.set_insert(else_bb);
        node.pElse->accept(*this);

        if (!m_builder.get_insert()->terminates())
            m_builder.build_jmp(siir::BlockAddress::get(m_cfg, merge_bb));
        
    }

    if (merge_bb->has_predecessors()) {
        m_func->push_back(merge_bb);
        m_builder.set_insert(merge_bb);
    }
}

void Codegen::visit(WhileStmt& node) {
    siir::BasicBlock* cond_bb = siir::BasicBlock::create({}, m_func);
    siir::BasicBlock* body_bb = siir::BasicBlock::create({});
    siir::BasicBlock* merge_bb = siir::BasicBlock::create({});

    m_builder.build_jmp(siir::BlockAddress::get(m_cfg, cond_bb));

    m_builder.set_insert(cond_bb);
    m_vctx = RValue;
    node.pCond->accept(*this);
    assert(m_tmp);

    m_builder.build_brif(
        inject_bool_cmp(m_tmp), 
        siir::BlockAddress::get(m_cfg, body_bb), 
        siir::BlockAddress::get(m_cfg, merge_bb));

    m_func->push_back(body_bb);
    m_builder.set_insert(body_bb);

    siir::BasicBlock* prev_cond = m_cond;
    siir::BasicBlock* prev_merge = m_merge;
    m_cond = cond_bb;
    m_merge = merge_bb;

    node.pBody->accept(*this);

    if (!m_builder.get_insert()->terminates())
        m_builder.build_jmp(siir::BlockAddress::get(m_cfg, cond_bb));

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

void Codegen::visit(Rune& node) {

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
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_Add, lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_FAdd, lhs, rhs);
    } else Logger::fatal(
        "unsupported '+' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_add_assign(const BinaryExpr& node) {
    
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
            m_builder.build_unop(rhs->get_type(), siir::UnopInst::UNOP_Neg, rhs));
    } else if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_Add, lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_FAdd, lhs, rhs);
    } else Logger::fatal(
        "unsupported '+' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_sub_assign(const BinaryExpr& node) {
    
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

    if (lhs->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_Mul, lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_FMul, lhs, rhs);
    } else Logger::fatal(
        "unsupported '*' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_mul_assign(const BinaryExpr& node) {
    
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
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_SDiv, lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_UDiv, lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_float()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_FDiv, lhs, rhs);
    } else Logger::fatal(
        "unsupported '/' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    ); 
}

void Codegen::codegen_binary_div_assign(const BinaryExpr& node) {
    
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
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_SRem, lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_URem, lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_float()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_FRem, lhs, rhs);
    } else Logger::fatal(
        "unsupported '/' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    ); 
}

void Codegen::codegen_binary_mod_assign(const BinaryExpr& node) {
    
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
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Eq, lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Oeq, lhs, rhs);
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
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Ne, lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_One, lhs, rhs);
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
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Slt, lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Ult, lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Olt, lhs, rhs);
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
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Sle, lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Ule, lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Ole, lhs, rhs);
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
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Sgt, lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Ugt, lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Ogt, lhs, rhs);
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
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Sge, lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Uge, lhs, rhs);
    } else if (lhs->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Oge, lhs, rhs);
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
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_And, lhs, rhs);
    } else Logger::fatal(
        "unsupported '&' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_bitwise_and_assign(const BinaryExpr& node) {
    
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
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_Or, lhs, rhs);
    } else Logger::fatal(
        "unsupported '|' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_bitwise_or_assign(const BinaryExpr& node) {
    
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
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_Xor, lhs, rhs);
    } else Logger::fatal(
        "unsupported '^' operator between on type '" + 
            node.get_type()->to_string() + "'",
        node.get_span() 
    );
}

void Codegen::codegen_binary_bitwise_xor_assign(const BinaryExpr& node) {
    
}

void Codegen::codegen_binary_logical_and(const BinaryExpr& node) {
    
}

void Codegen::codegen_binary_logical_or(const BinaryExpr& node) {
    
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
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_Shl, lhs, rhs);
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
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_Sar, lhs, rhs);
    } else if (node.get_lhs()->get_type()->is_unsigned_int()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), siir::BinopInst::BINOP_Shr, lhs, rhs);
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
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), 
            siir::BinopInst::BINOP_Add, 
            preop, 
            siir::ConstantInt::get(m_cfg, preop->get_type(), 1));
    } else if (preop->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), 
            siir::BinopInst::BINOP_FAdd, 
            preop, 
            siir::ConstantFP::get(m_cfg, preop->get_type(), 1.f));
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
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), 
            siir::BinopInst::BINOP_Sub, 
            preop, 
            siir::ConstantInt::get(m_cfg, preop->get_type(), 1));
    } else if (preop->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_binop(
            lower_type(node.get_type()), 
            siir::BinopInst::BINOP_FSub, 
            preop, 
            siir::ConstantFP::get(m_cfg, preop->get_type(), 1.f));
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
    m_vctx = LValue;
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

    siir::Value* value = m_tmp;

    if (value->get_type()->is_integer_type()) {
        m_tmp = m_builder.build_unop(
            lower_type(node.get_type()), siir::UnopInst::UNOP_Neg, value);
    } else if (value->get_type()->is_pointer_type()) {
        m_tmp = m_builder.build_unop(
            lower_type(node.get_type()), siir::UnopInst::UNOP_Neg, value);
    } else if (value->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_unop(
            lower_type(node.get_type()), siir::UnopInst::UNOP_FNeg, value);
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
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Eq, 
            value, 
            siir::ConstantInt::get(m_cfg, value->get_type(), 0));
    } else if (value->get_type()->is_floating_point_type()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Oeq, 
            value, 
            siir::ConstantFP::get(m_cfg, value->get_type(), 1.f));
    } else if (value->get_type()->is_pointer_type()) {
        m_tmp = m_builder.build_cmp(
            siir::CmpInst::CMP_Eq, 
            value, 
            siir::ConstantNull::get(m_cfg, value->get_type()));
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
        m_tmp = m_builder.build_unop(
            lower_type(node.get_type()), siir::UnopInst::UNOP_Not, m_tmp);
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
        if (auto constant = dynamic_cast<siir::ConstantInt*>(m_tmp)) {
            m_tmp = siir::ConstantInt::get(
                m_cfg, dst_type, constant->get_value());
            return;
        }

        siir::UnopInst::Ops op;
        if (src_sz > dst_sz) // Downcasting.
            op = siir::UnopInst::UNOP_Trunc;
        else if (node.get_expr()->get_type()->is_signed_int())
            op = siir::UnopInst::UNOP_SExt;
        else
            op = siir::UnopInst::UNOP_ZExt;

        m_tmp = m_builder.build_unop(dst_type, op, m_tmp);
    } else if (src_type->is_floating_point_type() 
      && dst_type->is_floating_point_type()) {
        // Floating point -> Floating point casts.
        if (src_sz == dst_sz)
            return;

        // Fold possible constants here if possible.
        if (auto constant = dynamic_cast<siir::ConstantFP*>(m_tmp)) {
            m_tmp = siir::ConstantFP::get(
                m_cfg, dst_type, constant->get_value());
            return;
        }

        siir::UnopInst::Ops op;
        if (src_sz > dst_sz) // Downcasting.
            op = siir::UnopInst::UNOP_FTrunc;
        else // Upcasting.
            op = siir::UnopInst::UNOP_FExt;

        m_tmp = m_builder.build_unop(dst_type, op, m_tmp);
    } else if (src_type->is_integer_type()
      && dst_type->is_floating_point_type()) {
        // Integer -> Floating point conversions.
        siir::UnopInst::Ops op;
        if (node.get_expr()->get_type()->is_signed_int())
            op = siir::UnopInst::UNOP_SI2FP;
        else if (node.get_expr()->get_type()->is_unsigned_int())
            op = siir::UnopInst::UNOP_UI2FP;

        m_tmp = m_builder.build_unop(dst_type, op, m_tmp);
    } else if (src_type->is_floating_point_type()
      && dst_type->is_integer_type()) {
        // Floating point -> Integer conversions.
        siir::UnopInst::Ops op;
        if (node.get_type()->is_signed_int())
            op = siir::UnopInst::UNOP_FP2SI;
        else if (node.get_type()->is_unsigned_int())
            op = siir::UnopInst::UNOP_FP2UI;

        m_tmp = m_builder.build_unop(dst_type, op, m_tmp);
    } else if (src_type->is_pointer_type() 
      && dst_type->is_pointer_type()) {
        // Pointer -> Pointer reinterpretations.
        m_tmp = m_builder.build_unop(
            dst_type, siir::UnopInst::UNOP_Reint, m_tmp);
    } else if (src_type->is_array_type()
      && dst_type->is_pointer_type()) {
        // Array -> Pointer decay.
        m_tmp = m_builder.build_unop(
            dst_type, siir::UnopInst::UNOP_Reint, m_tmp);
    } else if (src_type->is_integer_type()
      && dst_type->is_pointer_type()) {
        // Integer -> Pointer casts.
        m_tmp = m_builder.build_unop(
            dst_type, siir::UnopInst::UNOP_I2P, m_tmp);
    } else if (src_type->is_pointer_type()
      && dst_type->is_integer_type()) {
        // Pointer -> Integer casts.
        m_tmp = m_builder.build_unop(
            dst_type, siir::UnopInst::UNOP_P2I, m_tmp);
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

    // Resolve the referenced local in the current function.
    siir::Local* local = m_func->get_local(node.get_name());
    assert(local);

    m_tmp = local;

    if (m_vctx == RValue)
        m_tmp = m_builder.build_load(lower_type(node.get_type()), local);    
}

void Codegen::visit(MemberExpr& node) {

}

void Codegen::visit(CallExpr& node) {
    auto target = static_cast<const FunctionDecl*>(node.get_decl());
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

    m_builder.build_call(callee->get_type(), callee, args);
}

void Codegen::visit(RuneExpr& node) {

}
