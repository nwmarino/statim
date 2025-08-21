#include "ast.hpp"
#include "logger.hpp"
#include "translation_unit.hpp"
#include "bytecode.hpp"
#include "type.hpp"
#include "types.hpp"
#include "visitor.hpp"

#include <cassert>

using namespace stm;

static u32 get_type_size(const Type* type) {
    if (auto deferred = dynamic_cast<const DeferredType*>(type))
        return get_type_size(deferred->get_resolved());
    
    if (auto builtin = dynamic_cast<const BuiltinType*>(type)) {
        switch (builtin->get_kind()) {
        case BuiltinType::Kind::Void:
            return 0;
        case BuiltinType::Kind::Bool:
        case BuiltinType::Kind::Char:
        case BuiltinType::Kind::SInt8:
        case BuiltinType::Kind::UInt8:
            return 1;
        case BuiltinType::Kind::SInt16:
        case BuiltinType::Kind::UInt16:
            return 2;
        case BuiltinType::Kind::SInt32:
        case BuiltinType::Kind::UInt32:
        case BuiltinType::Kind::Float32:
            return 4;
        case BuiltinType::Kind::SInt64:
        case BuiltinType::Kind::UInt64:
        case BuiltinType::Kind::Float64:
            return 8;
        }
    } else if (dynamic_cast<const PointerType*>(type)) {
        return 8;
    } else if (auto st = dynamic_cast<const StructType*>(type)) {
        /// TODO: Adjust for padding and semantics.
        u32 size = 0;
        for (auto field : st->get_fields())
            size += get_type_size(field);

        return size;
    } else if (auto et = dynamic_cast<const EnumType*>(type)) {
        return get_type_size(et->get_underlying());
    } else {
        return 0;
    }
}

static Instruction::Size get_inst_size(const Type* type) {
    u32 size = get_type_size(type);
    switch (size) {
    case 1:
        return Instruction::Size::Byte;
    case 2:
        return Instruction::Size::Half;
    case 4:
        if (type->is_float())
            return Instruction::Size::Single;
        else
            return Instruction::Size::Quad;
    case 8:
        if (type->is_float())
            return Instruction::Size::Double;
        else
            return Instruction::Size::Word;
    }

    return Instruction::Size::None;
}

void Codegen::cgn_binary_assign(BinaryExpr& node) {
    vctx = ValueContext::LValue;
    node.pLeft->accept(*this);
    assert(tmp.has_value());

    Operand lhs = *tmp;
    tmp = std::nullopt;

    vctx = ValueContext::RValue;
    node.pRight->accept(*this);
    assert(tmp.has_value());

    Instruction::create(
        pInsert, 
        Opcode::Move, 
        { *tmp, lhs }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));
}

void Codegen::cgn_binary_add(BinaryExpr& node) {

}

void Codegen::cgn_binary_add_assign(BinaryExpr& node) {
    assert(false && "not implemented");
}

void Codegen::cgn_binary_sub(BinaryExpr& node) {

}

void Codegen::cgn_binary_sub_assign(BinaryExpr& node) {
    assert(false && "not implemented");
}

void Codegen::cgn_binary_mul(BinaryExpr& node) {

}

void Codegen::cgn_binary_mul_assign(BinaryExpr& node) {
    assert(false && "not implemented");
}

void Codegen::cgn_binary_div(BinaryExpr& node) {

}

void Codegen::cgn_binary_div_assign(BinaryExpr& node) {
    assert(false && "not implemented");
}

void Codegen::cgn_binary_mod(BinaryExpr& node) {

}

void Codegen::cgn_binary_mod_assign(BinaryExpr& node) {
    assert(false && "not implemented");
}

void Codegen::cgn_binary_eq(BinaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pLeft->accept(*this);
    assert(tmp.has_value());

    Operand lhs = *tmp;

    vctx = ValueContext::RValue;
    node.pRight->accept(*this);
    assert(tmp.has_value());

    Opcode opcode = Opcode::Cmpeq;
    if (node.get_lhs()->get_type()->is_float())
        opcode = Opcode::Cmpoeq;

    Instruction::create(
        pInsert, 
        opcode, 
        { lhs, *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = std::nullopt;
}

void Codegen::cgn_binary_neq(BinaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pLeft->accept(*this);
    assert(tmp.has_value());

    Operand lhs = *tmp;

    vctx = ValueContext::RValue;
    node.pRight->accept(*this);
    assert(tmp.has_value());

    Opcode opcode = Opcode::Cmpne;
    if (node.get_lhs()->get_type()->is_float())
        opcode = Opcode::Cmpone;

    Instruction::create(
        pInsert, 
        opcode, 
        { lhs, *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = std::nullopt;
}

void Codegen::cgn_binary_lt(BinaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pLeft->accept(*this);
    assert(tmp.has_value());

    Operand lhs = *tmp;

    vctx = ValueContext::RValue;
    node.pRight->accept(*this);
    assert(tmp.has_value());

    Opcode opcode;
    if (node.get_lhs()->get_type()->is_signed_int())
        opcode = Opcode::Cmpslt;
    else if (node.get_lhs()->get_type()->is_unsigned_int())
        opcode = Opcode::Cmpult;
    else if (node.get_lhs()->get_type()->is_float())
        opcode = Opcode::Cmpolt;
    else
        assert(false);

    Instruction::create(
        pInsert, 
        opcode, 
        { lhs, *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = std::nullopt;
}

void Codegen::cgn_binary_le(BinaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pLeft->accept(*this);
    assert(tmp.has_value());

    Operand lhs = *tmp;

    vctx = ValueContext::RValue;
    node.pRight->accept(*this);
    assert(tmp.has_value());

    Opcode opcode;
    if (node.get_lhs()->get_type()->is_signed_int())
        opcode = Opcode::Cmpsle;
    else if (node.get_lhs()->get_type()->is_unsigned_int())
        opcode = Opcode::Cmpule;
    else if (node.get_lhs()->get_type()->is_float())
        opcode = Opcode::Cmpole;
    else
        assert(false);

    Instruction::create(
        pInsert, 
        opcode, 
        { lhs, *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = std::nullopt;
}

void Codegen::cgn_binary_gt(BinaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pLeft->accept(*this);
    assert(tmp.has_value());

    Operand lhs = *tmp;

    vctx = ValueContext::RValue;
    node.pRight->accept(*this);
    assert(tmp.has_value());

    Opcode opcode;
    if (node.get_lhs()->get_type()->is_signed_int())
        opcode = Opcode::Cmpsgt;
    else if (node.get_lhs()->get_type()->is_unsigned_int())
        opcode = Opcode::Cmpugt;
    else if (node.get_lhs()->get_type()->is_float())
        opcode = Opcode::Cmpogt;
    else
        assert(false);

    Instruction::create(
        pInsert, 
        opcode, 
        { lhs, *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = std::nullopt;
}

void Codegen::cgn_binary_ge(BinaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pLeft->accept(*this);
    assert(tmp.has_value());

    Operand lhs = *tmp;

    vctx = ValueContext::RValue;
    node.pRight->accept(*this);
    assert(tmp.has_value());

    Opcode opcode;
    if (node.get_lhs()->get_type()->is_signed_int())
        opcode = Opcode::Cmpsge;
    else if (node.get_lhs()->get_type()->is_unsigned_int())
        opcode = Opcode::Cmpuge;
    else if (node.get_lhs()->get_type()->is_float())
        opcode = Opcode::Cmpoge;
    else
        assert(false);

    Instruction::create(
        pInsert, 
        opcode, 
        { lhs, *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = std::nullopt;
}

void Codegen::cgn_binary_band(BinaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pLeft->accept(*this);
    assert(tmp.has_value());

    Operand lhs = *tmp;

    vctx = ValueContext::RValue;
    node.pRight->accept(*this);
    assert(tmp.has_value());

    Instruction::create(
        pInsert, 
        Opcode::And, 
        { lhs, *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = std::nullopt;
}

void Codegen::cgn_binary_band_assign(BinaryExpr& node) {
    
}

void Codegen::cgn_binary_bor(BinaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pLeft->accept(*this);
    assert(tmp.has_value());

    Operand lhs = *tmp;

    vctx = ValueContext::RValue;
    node.pRight->accept(*this);
    assert(tmp.has_value());

    Instruction::create(
        pInsert, 
        Opcode::Or, 
        { lhs, *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = std::nullopt;
}

void Codegen::cgn_binary_bor_assign(BinaryExpr& node) {

}

void Codegen::cgn_binary_bxor(BinaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pLeft->accept(*this);
    assert(tmp.has_value());

    Operand lhs = *tmp;

    vctx = ValueContext::RValue;
    node.pRight->accept(*this);
    assert(tmp.has_value());

    Instruction::create(
        pInsert, 
        Opcode::Xor, 
        { lhs, *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = std::nullopt;
}

void Codegen::cgn_binary_bxor_assign(BinaryExpr& node) {

}

void Codegen::cgn_binary_land(BinaryExpr& node) {

}

void Codegen::cgn_binary_lor(BinaryExpr& node) {

}

void Codegen::cgn_binary_ls(BinaryExpr& node) {
    assert(false && "not implemented");
}

void Codegen::cgn_binary_ls_assign(BinaryExpr& node) {
    assert(false && "not implemented");
}

void Codegen::cgn_binary_rs(BinaryExpr& node) {
    assert(false && "not implemented");
}

void Codegen::cgn_binary_rs_assign(BinaryExpr& node) {
    assert(false && "not implemented");
}

void Codegen::cgn_unary_inc(UnaryExpr& node) {
    ValueContext node_vc = vctx;
    const Type* expr_type = node.get_expr()->get_type();

    // First, get the inner value as an lvalue for later storage.
    vctx = ValueContext::LValue;
    node.pExpr->accept(*this);
    assert(tmp.has_value());

    // Load the value to a register to be used as an rvalue.
    Operand rval { Register(vreg++) };
    Instruction::create(
        pInsert, 
        Opcode::Move, 
        { *tmp, rval }, 
        Metadata(node.get_span()),
        get_inst_size(expr_type));

    if (node.is_postfix()) {
        // If the increment is a postfix, then the original value is still
        // needed, and since it'll be overwritten by the increment, its copied
        // here.
        Operand copy { Register(vreg++) };
        Instruction::create(
            pInsert, 
            Opcode::Copy, 
            { rval, copy }, 
            Metadata(node.get_span()));

        tmp = copy;
    } else {
        tmp = rval;
    }
    
    if (expr_type->is_int() || expr_type->is_float()) {
        // Simple increments can be done straight to the value.
        Instruction::create(
            pInsert, 
            Opcode::Inc, 
            { rval }, 
            Metadata(node.get_span()),
            get_inst_size(expr_type));
    } else if (expr_type->is_pointer()) {
        // For pointer increments, we need to add the byte size of the pointee
        // to the original address.
        const Type* pointee = expr_type->as_pointer()->get_pointee();
        Operand size { Register(vreg++) };
        Instruction::create(
            pInsert, 
            Opcode::Constant, 
            { get_type_size(pointee), size }, 
            Metadata(node.get_span()),
            Instruction::Size::Word);
        
        Instruction::create(
            pInsert, 
            Opcode::Add, 
            { size, rval }, 
            Metadata(node.get_span()),
            Instruction::Size::Word);
    } else Logger::fatal(
        "unsupported type for '++' operator: '" + expr_type->to_string() + "'", 
        node.get_span()
    );

    // Store the result of the increment.
    Instruction::create(
        pInsert, 
        Opcode::Move, 
        { rval, *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(expr_type));
}

void Codegen::cgn_unary_dec(UnaryExpr& node) {
    assert(false && "not implemented");
}

void Codegen::cgn_unary_deref(UnaryExpr& node) {
    ValueContext node_vc = vctx;
    vctx = ValueContext::RValue;
    node.pExpr->accept(*this);
    assert(tmp.has_value());

    if (node_vc == ValueContext::RValue) {
        Operand dst { Register(vreg++) };
        Instruction::create(
            pInsert, 
            Opcode::Move, 
            { *tmp, dst },
            Metadata(node.get_span()));

        tmp = dst;
    }
}

void Codegen::cgn_unary_addrof(UnaryExpr& node) {
    vctx = ValueContext::LValue;
    node.pExpr->accept(*this);
    assert(tmp.has_value());
}

void Codegen::cgn_unary_neg(UnaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pExpr->accept(*this);
    assert(tmp.has_value());
    assert(tmp->kind == Operand::Kind::Register);

    Instruction::create(
        pInsert, 
        Opcode::Neg, 
        { *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));
}

void Codegen::cgn_unary_lnot(UnaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pExpr->accept(*this);
    assert(tmp.has_value());

    Operand zero { Register(vreg++) };
    Instruction::create(
        pInsert, 
        Opcode::Xor, 
        { zero, *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_expr()->get_type()));
}

void Codegen::cgn_unary_bnot(UnaryExpr& node) {
    vctx = ValueContext::RValue;
    node.pExpr->accept(*this);
    assert(tmp.has_value());
    assert(tmp->kind == Operand::Kind::Register);

    Instruction::create(
        pInsert, 
        Opcode::Not, 
        { *tmp }, 
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));
}

void Codegen::run(TranslationUnit& unit) {
    frame = std::make_unique<Frame>(root.get_file());
    root.accept(*this);
    unit.set_frame(std::move(frame));
    frame = nullptr;
}

void Codegen::visit(Root& node) {
    phase = Phase::Declare;
    for (auto decl : node.decls) decl->accept(*this);

    phase = Phase::Define;
    for (auto decl : node.decls) decl->accept(*this);
}

void Codegen::visit(FunctionDecl& node) {
    if (phase == Phase::Declare) {
        Function* function = new Function(node.get_name());
        frame->add_function(function);

        for (auto param : node.params) {
            StackSlot* slot = new StackSlot(
                param->get_name(),
                function->get_stack_size() + get_type_size(param->get_type()),
                function);
        }
    } else if (phase == Phase::Define) {
        pFunction = frame->get_function(node.get_name());
        
        BasicBlock* entry = new BasicBlock(pFunction);
        pInsert = entry;

        for (i64 idx = 0, e = node.num_params(); idx != e; ++idx) {
            StackSlot* slot = pFunction->get_slot(node.params[idx]->get_name());

            Instruction::create(
                pInsert, 
                Opcode::Move, 
                { ArgumentRef(idx), MemoryRef(Register(0), slot->get_offset()) }, 
                Metadata(node.get_span()),
                get_inst_size(node.params[idx]->get_type()));
        }

        node.pBody->accept(*this);

        if (!pInsert->terminates()) {
            if (node.get_type()->get_return_type()->is_void()) {
                Instruction::create(
                    pInsert, 
                    Opcode::Return, 
                    {}, 
                    Metadata(node.get_body()->get_span().end));
            } else Logger::fatal(
                "function '" + node.get_name() + "' does not always return",
                node.get_span()
            );
        }
        
        pInsert = nullptr;
        pFunction = nullptr;
    }
}

void Codegen::visit(VariableDecl& node) {
    StackSlot* slot = new StackSlot(
        node.get_name(), 
        pFunction->get_stack_size() + get_type_size(node.get_type()),
        pFunction);

    if (node.has_init()) {
        vctx = ValueContext::RValue;
        node.pInit->accept(*this);
        assert(tmp.has_value() && "variable initializer does not produce a value");

        Operand src = *tmp;
        Operand dst = Operand(MemoryRef(Register(0), slot->get_offset()));
        tmp = std::nullopt;

        Instruction::create(
            pInsert, 
            Opcode::Move, 
            { src, dst }, 
            Metadata(node.get_span()), 
            get_inst_size(node.get_type()));
    }
}

void Codegen::visit(FieldDecl& node) {

}

void Codegen::visit(StructDecl& node) {

}

void Codegen::visit(EnumValueDecl& node) {

}

void Codegen::visit(EnumDecl& node) {

}

void Codegen::visit(BlockStmt& node) {
    for (auto stmt : node.stmts) stmt->accept(*this);
}

void Codegen::visit(BreakStmt& node) {
    assert(pMerge && "no merge for break statement to branch to");

    Instruction::create(
        pInsert, 
        Opcode::Jump, 
        { BlockRef(pMerge) }, 
        Metadata(node.get_span()));
}

void Codegen::visit(ContinueStmt& node) {
    assert(pCond && "no merge for continue statement to branch to");
    
    Instruction::create(
        pInsert, 
        Opcode::Jump, 
        { BlockRef(pCond) }, 
        Metadata(node.get_span()));
}

void Codegen::visit(DeclStmt& node) {
    node.pDecl->accept(*this);
}

void Codegen::visit(IfStmt& node) {
    vctx = ValueContext::RValue;
    node.pCond->accept(*this);
    assert(!tmp.has_value() && "if condition does not produce comparison");

    BasicBlock* then_bb = new BasicBlock(pFunction);
    BasicBlock* else_bb = nullptr;
    BasicBlock* merge_bb = new BasicBlock();

    BasicBlock* branch_dst = merge_bb;
    if (node.has_else()) {
        else_bb = new BasicBlock();
        branch_dst = else_bb;
    }

    Instruction::create(
        pInsert, 
        Opcode::BranchFalse, 
        { BlockRef(branch_dst) }, 
        Metadata(node.get_span()));

    Instruction::create(
        pInsert, 
        Opcode::Jump, 
        { BlockRef(then_bb) }, 
        Metadata(node.get_cond()->get_span().end));

    pInsert = then_bb;
    node.pThen->accept(*this);

    if (!pInsert->terminates()) {
        Instruction::create(
            pInsert, 
            Opcode::Jump, 
            { BlockRef(merge_bb) }, 
            Metadata(node.get_then()->get_span().end));
    }

    if (node.has_else()) {
        pFunction->append(else_bb);
        pInsert = else_bb;
        node.pElse->accept(*this);

        if (!pInsert->terminates()) {
            Instruction::create(
                pInsert, 
                Opcode::Jump, 
                { BlockRef(merge_bb)}, 
                Metadata(node.get_else()->get_span().end));
        }
    }

    pFunction->append(merge_bb);
    pInsert = merge_bb;
}

void Codegen::visit(WhileStmt& node) {
    BasicBlock* cond_bb = new BasicBlock(pFunction);
    BasicBlock* body_bb = new BasicBlock();
    BasicBlock* merge_bb = new BasicBlock();

    Instruction::create(
        pInsert, 
        Opcode::Jump, 
        { BlockRef(cond_bb) }, 
        Metadata(node.get_span().begin));

    pInsert = cond_bb;
    vctx = ValueContext::RValue;
    node.pCond->accept(*this);
    assert(!tmp.has_value() && "while condition does not produce comparison");

    Instruction::create(
        pInsert, 
        Opcode::BranchTrue, 
        { BlockRef(merge_bb) }, 
        Metadata(node.get_cond()->get_span().begin));

    Instruction::create(
        pInsert, 
        Opcode::Jump, 
        { BlockRef(body_bb) }, 
        Metadata(node.get_cond()->get_span().end));

    pFunction->append(body_bb);
    pInsert = body_bb;
    
    BasicBlock* prev_cond = pCond;
    BasicBlock* prev_merge = pMerge;
    pCond = cond_bb;
    pMerge = merge_bb;

    node.pBody->accept(*this);

    if (!pInsert->terminates()) {
        Instruction::create(
            pInsert, 
            Opcode::Jump, 
            { BlockRef(cond_bb) }, 
            Metadata(node.get_body()->get_span().end));
    }

    pFunction->append(merge_bb);
    pInsert = merge_bb;
    pCond = prev_cond;
    pMerge = prev_merge;
}

void Codegen::visit(RetStmt& node) {
    std::vector<Operand> operands;

    if (node.has_expr()) {
        vctx = ValueContext::RValue;
        node.pExpr->accept(*this);
        assert(tmp.has_value());
        operands.push_back(*tmp);
        tmp = std::nullopt;
    }

    Instruction::create(
        pInsert, 
        Opcode::Return, 
        operands, 
        Metadata(node.get_span()));
}

void Codegen::visit(Rune& node) {

}

void Codegen::visit(BoolLiteral& node) {
    Operand src { static_cast<i64>(node.get_value()) };
    Operand dst { Register(vreg++) };

    Instruction::create(
        pInsert, 
        Opcode::Constant, 
        { src, dst },
        Metadata(node.get_span()),
        Instruction::Size::Byte);

    tmp = dst;
}

void Codegen::visit(IntegerLiteral& node) {
    Operand src { static_cast<i64>(node.get_value()) };
    Operand dst { Register(vreg++) };

    Instruction::create(
        pInsert,
        Opcode::Constant,
        { src, dst },
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = dst;
}

void Codegen::visit(FloatLiteral& node) {
    Operand src { node.get_value() };
    Operand dst { Register(vreg++) };

    Instruction::create(
        pInsert,
        Opcode::Constant,
        { src, dst },
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = dst;
}

void Codegen::visit(CharLiteral& node) {
    Operand src { static_cast<i64>(node.get_value()) };
    Operand dst { Register(vreg++) };

    Instruction::create(
        pInsert,
        Opcode::Constant,
        { src, dst },
        Metadata(node.get_span()),
        Instruction::Size::Byte);

    tmp = dst;
}

void Codegen::visit(StringLiteral& node) {
    Operand src { node.get_value().data() };
    Operand dst { Register(vreg++) };

    tmp = dst;
}

void Codegen::visit(NullLiteral& node) {
    Operand src { static_cast<i64>(0) };
    Operand dst { Register(vreg++) };

    Instruction::create(
        pInsert, 
        Opcode::Constant, 
        { src, dst }, 
        Metadata(node.get_span()),
        Instruction::Size::Word);

    tmp = dst;
}

void Codegen::visit(BinaryExpr& node) {
    switch (node.get_operator()) {
    case BinaryExpr::Operator::Assign:
        return cgn_binary_assign(node);
    case BinaryExpr::Operator::Add:
        return cgn_binary_add(node);
    case BinaryExpr::Operator::Add_Assign:
        return cgn_binary_add_assign(node);
    case BinaryExpr::Operator::Sub:
        return cgn_binary_sub(node);
    case BinaryExpr::Operator::Sub_Assign:
        return cgn_binary_sub_assign(node);
    case BinaryExpr::Operator::Mul:
        return cgn_binary_mul(node);
    case BinaryExpr::Operator::Mul_Assign:
        return cgn_binary_mul_assign(node);
    case BinaryExpr::Operator::Div:
        return cgn_binary_div(node);
    case BinaryExpr::Operator::Div_Assign:
        return cgn_binary_div_assign(node);
    case BinaryExpr::Operator::Mod:
        return cgn_binary_mod(node);
    case BinaryExpr::Operator::Mod_Assign:
        return cgn_binary_mod_assign(node);
    case BinaryExpr::Operator::Equals:
        return cgn_binary_eq(node);
    case BinaryExpr::Operator::Not_Equals:
        return cgn_binary_neq(node);
    case BinaryExpr::Operator::Less_Than:
        return cgn_binary_lt(node);
    case BinaryExpr::Operator::Less_Than_Equals:
        return cgn_binary_le(node);
    case BinaryExpr::Operator::Greater_Than:
        return cgn_binary_gt(node);
    case BinaryExpr::Operator::Greater_Than_Equals:
        return cgn_binary_ge(node);
    case BinaryExpr::Operator::Bitwise_And:
        return cgn_binary_band(node);
    case BinaryExpr::Operator::Bitwise_And_Assign:
        return cgn_binary_band_assign(node);
    case BinaryExpr::Operator::Bitwise_Or:
        return cgn_binary_bor(node);
    case BinaryExpr::Operator::Bitwise_Or_Assign:
        return cgn_binary_bor_assign(node);
    case BinaryExpr::Operator::Bitwise_Xor:
        return cgn_binary_bxor(node);
    case BinaryExpr::Operator::Bitwise_Xor_Assign:
        return cgn_binary_bxor_assign(node);
    case BinaryExpr::Operator::Logical_And:
        return cgn_binary_land(node);
    case BinaryExpr::Operator::Logical_Or:
        return cgn_binary_lor(node);
    case BinaryExpr::Operator::Left_Shift:
        return cgn_binary_ls(node);
    case BinaryExpr::Operator::Left_Shift_Assign:
        return cgn_binary_ls_assign(node);
    case BinaryExpr::Operator::Right_Shift:
        return cgn_binary_rs(node);
    case BinaryExpr::Operator::Right_Shift_Assign:
        return cgn_binary_rs_assign(node);
    default:
        assert(false);
    }
}

void Codegen::visit(UnaryExpr& node) {
    switch (node.get_operator()) {
    case UnaryExpr::Operator::Increment:
        return cgn_unary_inc(node);
    case UnaryExpr::Operator::Decrement:
        return cgn_unary_dec(node);
    case UnaryExpr::Operator::Dereference:
        return cgn_unary_deref(node);
    case UnaryExpr::Operator::Address_Of:
        return cgn_unary_addrof(node);
    case UnaryExpr::Operator::Negate:
        return cgn_unary_neg(node);
    case UnaryExpr::Operator::Logical_Not:
        return cgn_unary_lnot(node);
    case UnaryExpr::Operator::Bitwise_Not:
        return cgn_unary_bnot(node);
    default:
        assert(false);
    }
}

void Codegen::visit(CastExpr& node) {
    vctx = ValueContext::RValue;
    node.pExpr->accept(*this);
    assert(tmp.has_value());

    const Type* src_type = node.get_expr()->get_type();
    const Type* dst_type = node.get_type();

    if (src_type->is_deferred())
        src_type = src_type->as_deferred()->get_resolved();
    if (dst_type->is_deferred())
        dst_type = dst_type->as_deferred()->get_resolved();
}

void Codegen::visit(ParenExpr& node) {
    node.pExpr->accept(*this);
}

void Codegen::visit(SizeofExpr& node) {
    Operand constant { static_cast<i64>(get_type_size(node.get_target())) };
    Operand dst { Register(vreg++) };

    Instruction::create(
        pInsert,
        Opcode::Constant,
        { constant, dst },
        Metadata(node.get_span()),
        get_inst_size(node.get_type()));

    tmp = dst;
}

void Codegen::visit(SubscriptExpr& node) {
    ValueContext node_vc = vctx;

    vctx = ValueContext::LValue;
    node.pBase->accept(*this);
    assert(tmp.has_value());

    Operand addr = *tmp;

    // Get the offset of the index as the result of it multiplied by the
    // base size.
    vctx = ValueContext::RValue;
    node.pIndex->accept(*this);
    assert(tmp.has_value());

    Operand size { Register(vreg++) };
    Instruction::create(
        pInsert, 
        Opcode::Constant, 
        { Immediate(static_cast<i64>(get_type_size(node.get_type()))), size }, 
        Metadata(node.get_span()),
        Instruction::Size::Word);

    Instruction::create(
        pInsert, 
        Opcode::Mul, 
        { size, *tmp },
        Metadata(node.get_span()),
        Instruction::Size::Word);

    Instruction::create(
        pInsert, 
        Opcode::Add, 
        { *tmp, addr },
        Metadata(node.get_span()),
        Instruction::Size::Word);

    if (node_vc == ValueContext::RValue) {
        Operand src { MemoryRef(addr.reg, 0) };
        Operand dst { Register(vreg++) };
        Instruction::create(
            pInsert, 
            Opcode::Move, 
            { src, dst },
            Metadata(node.get_span()),
            get_inst_size(node.get_type()));

        tmp = dst;
    } else {
        tmp = addr;
    }
}

void Codegen::visit(ReferenceExpr& node) {
    if (auto value = dynamic_cast<const EnumValueDecl*>(node.get_decl())) {
        // If the referenced declaration is an enum value, then it can
        // resolved at this point to it's integer value.
        Immediate imm { value->get_value() };
        Operand dst { Register(vreg++) };
        Instruction::create(
            pInsert, 
            Opcode::Constant, 
            { imm, dst },
            Metadata(node.get_span()),
            get_inst_size(value->get_type()));

        tmp = dst;
        return;
    }

    // Resolve the referenced variable in the function's stack frame.
    StackSlot* slot = pFunction->get_slot(node.get_name());
    assert(slot && "slot does not exist in function");

    if (vctx == ValueContext::LValue) {
        // If the reference is being used as an lvalue, i.e. `x = ...`, then
        // it's best to load the stack address into a register.

        Operand src { MemoryRef(Register(0), slot->get_offset()) };
        Operand dst { Register(vreg++) };

        Instruction::create(
            pInsert, 
            Opcode::Lea, 
            { src, dst }, 
            Metadata(node.get_span()));

        tmp = dst;
    } else if (vctx == ValueContext::RValue) {
        // If the reference is being used as an rvalue, i.e. `... = x`, then
        // its dereferenced value should get loaded into a register.

        Operand src { MemoryRef(Register(0), slot->get_offset()) };
        Operand dst { Register(vreg++) };

        Instruction::create(
            pInsert, 
            Opcode::Move, 
            { src, dst }, 
            Metadata(node.get_span()),
            get_inst_size(node.get_type()));

        tmp = dst;
    }
}

void Codegen::visit(MemberExpr& node) {
    auto node_vc = vctx;
    auto target = static_cast<const FieldDecl*>(node.get_decl());
    auto structure = target->get_parent();

    // Determine the offset of the target field in the base structure.
    u32 offset = 0;
    for (auto field : structure->get_fields()) {
        if (field == target) break;
        offset += get_type_size(field->get_type());
    }

    const Type* base_type = node.get_base()->get_type();
    if (base_type->is_struct()) {
        vctx = ValueContext::LValue;
        node.pBase->accept(*this);
        assert(tmp.has_value());
        assert(tmp->kind == Operand::Kind::Register);

        tmp->kind = Operand::Kind::Memory;
        tmp->mem.base = tmp->reg;
        tmp->mem.offset = offset;
        //tmp->mem.offset += offset;
    } else if (base_type->is_pointer()) {
        vctx = ValueContext::RValue;
        node.pBase->accept(*this);
        assert(tmp.has_value());
        assert(tmp->kind == Operand::Kind::Register);

        tmp->kind = Operand::Kind::Memory;
        tmp->mem.base = tmp->reg;
        tmp->mem.offset = offset;
    } else Logger::fatal(
        "cannot apply '.' operator to base with type '" + 
            base_type->to_string() + "'",
        node.get_span()
    );

    if (node_vc == ValueContext::RValue) {
        // If the member access should be an r-value, then we should load the
        // value into a register and result in that.
        Operand dst { Register(vreg++) };
        Instruction::create(
            pInsert,
            Opcode::Move,
            { *tmp, dst },
            Metadata(node.get_span()),
            get_inst_size(node.get_type()));

        tmp = dst;
    }
}

void Codegen::visit(CallExpr& node) {

}

void Codegen::visit(RuneExpr& node) {

}
