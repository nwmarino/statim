#include "ast.hpp"
#include "translation_unit.hpp"
#include "bytecode.hpp"
#include "type.hpp"
#include "visitor.hpp"

using namespace stm;

static ValueType lower_type(const Type* pType) {
    if (auto builtin = dynamic_cast<const BuiltinType*>(pType)) {
        switch (builtin->get_kind()) {
        case BuiltinType::Kind::Void:
            return ValueType::None;
        case BuiltinType::Kind::Bool:
        case BuiltinType::Kind::Char:
        case BuiltinType::Kind::SInt8:
        case BuiltinType::Kind::UInt8:
            return ValueType::Int8;
        case BuiltinType::Kind::SInt16:
        case BuiltinType::Kind::UInt16:
            return ValueType::Int16;
        case BuiltinType::Kind::SInt32:
        case BuiltinType::Kind::UInt32:
            return ValueType::Int32;
        case BuiltinType::Kind::SInt64:
        case BuiltinType::Kind::UInt64:
            return ValueType::Int64;
        case BuiltinType::Kind::Float32:
            return ValueType::Float32;
        case BuiltinType::Kind::Float64:
            return ValueType::Float64;
        }
    } else if (auto deferred = dynamic_cast<const DeferredType*>(pType)) {
        return lower_type(deferred->get_resolved());
    } else if (auto ptr = dynamic_cast<const PointerType*>(pType)) {
        return ValueType::Pointer;
    }
    
    return ValueType::None;
}

static u32 get_type_size_in_bytes(ValueType type) {
    switch (type) {
    case ValueType::None:
        return 0;
    case ValueType::Int8:
        return 1;
    case ValueType::Int16:
        return 2;
    case ValueType::Int32:
    case ValueType::Float32:
        return 4;
    case ValueType::Int64:
    case ValueType::Float64:
    case ValueType::Pointer:
        return 8;
    }
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
        auto function_type = node.get_type();
        ValueType return_type = lower_type(function_type->get_return_type());
        std::vector<ValueType> param_types { node.num_params() };
        for (auto param : function_type->get_param_types())
            param_types.push_back(lower_type(param));

        Function* function = new Function(
            node.get_name(), param_types, return_type);

        frame->add_function(function);
    } else if (phase == Phase::Define) {
        pFunction = frame->get_function(node.get_name());
        
        BasicBlock* entry = new BasicBlock(pFunction);
        pInsert = entry;

        node.pBody->accept(*this);
        
        pInsert = nullptr;
        pFunction = nullptr;
    }
}

void Codegen::visit(VariableDecl& node) {
    StackSlot* slot = new StackSlot(
        node.get_name(), 
        pFunction->get_stack_size() + get_type_size_in_bytes(lower_type(node.get_type())),
        pFunction);

    if (node.has_init()) {
        vctx = ValueContext::RValue;
        node.pInit->accept(*this);
        assert(tmp.has_value() && "variable initializer does not produce a value");

        Operand src = *tmp;
        Operand dst = Operand(MemoryRef(Register(0), slot->get_offset()));
        tmp = std::nullopt;

        new Instruction(
            Instruction::Opcode::Move,
            { src, dst },
            Metadata(node.get_span()),
            {
                .size = get_type_size_in_bytes(lower_type(node.get_type()))
            },
            pInsert);
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

    new Instruction(
        Instruction::Opcode::Branch,
        { Operand(BlockRef(pMerge)) },
        Metadata(node.get_span()),
        {},
        pInsert);
}

void Codegen::visit(ContinueStmt& node) {
    assert(pCond && "no merge for continue statement to branch to");

    new Instruction(
        Instruction::Opcode::Branch,
        { Operand(BlockRef(pCond)) },
        Metadata(node.get_span()),
        {},
        pInsert);
}

void Codegen::visit(DeclStmt& node) {
    node.pDecl->accept(*this);
}

void Codegen::visit(IfStmt& node) {
    vctx = ValueContext::RValue;
    node.pCond->accept(*this);
    assert(tmp.has_value() && "if condition does not produce a value");

    BasicBlock* then_bb = new BasicBlock(pFunction);
    BasicBlock* else_bb = nullptr;
    BasicBlock* merge_bb = new BasicBlock();

    if (node.has_else()) {
        else_bb = new BasicBlock();
        new Instruction(
            Instruction::Opcode::Branch,
            { 
                *tmp, 
                Operand(BlockRef(then_bb)), 
                Operand(BlockRef(else_bb)) 
            },
            Metadata(node.get_span()),
            {},
            pInsert);
    } else {
        new Instruction(
            Instruction::Opcode::Branch,
            {
                *tmp,
                Operand(BlockRef(then_bb)),
                Operand(BlockRef(merge_bb))
            },
            Metadata(node.get_span()),
            {},
            pInsert);
    }

    pInsert = then_bb;
    node.pThen->accept(*this);

    if (!pInsert->terminates()) {
        new Instruction(
            Instruction::Opcode::Jump,
            { BlockRef(merge_bb) },
            Metadata(node.get_then()->get_span().end),
            {},
            pInsert);
    }

    if (node.has_else()) {
        pFunction->append(else_bb);
        pInsert = else_bb;
        node.pElse->accept(*this);

        if (!pInsert->terminates()) {
            new Instruction(
                Instruction::Opcode::Jump,
                { BlockRef(merge_bb) },
                Metadata(node.get_else()->get_span().end),
                {},
                pInsert);
        }
    }

    pFunction->append(merge_bb);
    pInsert = merge_bb;
}

void Codegen::visit(WhileStmt& node) {
    BasicBlock* cond_bb = new BasicBlock(pFunction);
    BasicBlock* body_bb = new BasicBlock();
    BasicBlock* merge_bb = new BasicBlock();

    new Instruction(
        Instruction::Opcode::Jump,
        { BlockRef(cond_bb) },
        Metadata(node.get_span().begin),
        {},
        pInsert);

    pInsert = cond_bb;
    vctx = ValueContext::RValue;
    node.pCond->accept(*this);
    assert(tmp.has_value() && "while condition does not produce a value");

    new Instruction(
        Instruction::Opcode::Branch,
        { *tmp, BlockRef(body_bb), BlockRef(merge_bb) },
        Metadata(node.get_cond()->get_span().begin),
        {},
        pInsert);

    pFunction->append(body_bb);
    pInsert = body_bb;
    
    BasicBlock* prev_cond = pCond;
    BasicBlock* prev_merge = pMerge;
    pCond = cond_bb;
    pMerge = merge_bb;

    node.pBody->accept(*this);

    if (!pInsert->terminates()) {
        new Instruction(
            Instruction::Opcode::Jump,
            { BlockRef(cond_bb) },
            Metadata(node.get_body()->get_span().end),
            {},
            pInsert);
    }

    pFunction->append(merge_bb);
    pInsert = merge_bb;
    pCond = prev_cond;
    prev_merge = prev_merge;
}

void Codegen::visit(RetStmt& node) {
    std::vector<Operand> operands;

    if (node.has_expr()) {
        vctx = ValueContext::RValue;
        node.pExpr->accept(*this);
        assert(tmp.has_value() && "return expression does not produce a value");
        operands.push_back(*tmp);
        tmp = std::nullopt;
    }

    new Instruction(
        Instruction::Opcode::Return, 
        operands,
        Metadata(node.get_span()), 
        {},
        pInsert);
}

void Codegen::visit(Rune& node) {

}

void Codegen::visit(BoolLiteral& node) {
    Operand src { static_cast<i64>(node.get_value()) };
    Operand dst { Register(vreg++) };

    new Instruction(
        Instruction::Opcode::Constant,
        { src, dst },
        Metadata(node.get_span()),
        {},
        pInsert);

    tmp = dst;
}

void Codegen::visit(IntegerLiteral& node) {
    Operand src { static_cast<i64>(node.get_value()) };
    Operand dst { Register(vreg++) };

    new Instruction(
        Instruction::Opcode::Constant,
        { src, dst },
        Metadata(node.get_span()),
        {},
        pInsert);

    tmp = dst;
}

void Codegen::visit(FloatLiteral& node) {
    Operand src { node.get_value() };
    Operand dst { Register(vreg++) };

    new Instruction(
        Instruction::Opcode::Float,
        { src, dst },
        Metadata(node.get_span()),
        {},
        pInsert);

    tmp = dst;
}

void Codegen::visit(CharLiteral& node) {
    Operand src { static_cast<i64>(node.get_value()) };
    Operand dst { Register(vreg++) };

    new Instruction(
        Instruction::Opcode::Constant,
        { src, dst },
        Metadata(node.get_span()),
        {},
        pInsert);

    tmp = dst;
}

void Codegen::visit(StringLiteral& node) {
    Operand src { node.get_value().data() };
    Operand dst { Register(vreg++) };

    new Instruction(
        Instruction::Opcode::String,
        { src, dst },
        Metadata(node.get_span()),
        {},
        pInsert);

    tmp = dst;
}

void Codegen::visit(NullLiteral& node) {
    Operand src { static_cast<i64>(0) };
    Operand dst { Register(vreg++) };

    new Instruction(
        Instruction::Opcode::String,
        { src, dst },
        Metadata(node.get_span()),
        {},
        pInsert);

    tmp = dst;
}

void Codegen::visit(BinaryExpr& node) {
    
}

void Codegen::visit(UnaryExpr& node) {

}

void Codegen::visit(CastExpr& node) {

}

void Codegen::visit(ParenExpr& node) {
    node.pExpr->accept(*this);
}

void Codegen::visit(SizeofExpr& node) {

}

void Codegen::visit(SubscriptExpr& node) {

}

void Codegen::visit(ReferenceExpr& node) {
    StackSlot* slot = pFunction->get_slot(node.get_name());
    assert(slot && "slot does not exist in function");

    if (vctx == ValueContext::LValue) {
        Operand src = Operand(StackRef(slot->get_offset()));
        Operand dst = Operand(Register(vreg++));

        new Instruction(
            Instruction::Opcode::Lea,
            { src, dst },
            Metadata(node.get_span()),
            {},
            pInsert);

        tmp = dst;
    } else if (vctx == ValueContext::RValue) {
        Operand src = Operand(MemoryRef(Register(0), slot->get_offset()));
        Operand dst = Operand(Register(vreg++));

        new Instruction(
            Instruction::Opcode::Move,
            { src, dst },
            Metadata(node.get_span()),
            { 
                .size = get_type_size_in_bytes(lower_type(node.get_type())) 
            },
            pInsert);

        tmp = dst;
    }
}

void Codegen::visit(MemberExpr& node) {

}

void Codegen::visit(CallExpr& node) {

}

void Codegen::visit(RuneExpr& node) {

}
