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
        instr = 0;
        
        BasicBlock* entry = new BasicBlock(pFunction);
        pInsert = entry;

        node.pBody->accept(*this);
        
        pInsert = nullptr;
        pFunction = nullptr;
    }
}

void Codegen::visit(ParameterDecl& node) {

}

void Codegen::visit(VariableDecl& node) {

}

void Codegen::visit(BlockStmt& node) {
    for (auto stmt : node.stmts) stmt->accept(*this);
}

void Codegen::visit(BreakStmt& node) {

}

void Codegen::visit(ContinueStmt& node) {

}

void Codegen::visit(DeclStmt& node) {
    node.pDecl->accept(*this);
}

void Codegen::visit(IfStmt& node) {

}

void Codegen::visit(WhileStmt& node) {

}

void Codegen::visit(RetStmt& node) {
    const Span& span = node.get_span();
    
    std::vector<Operand> operands;

    if (node.has_expr()) {
        node.pExpr->accept(*this);
        assert(tmp.has_value() && "return expression does not produce a value");
        operands.push_back(*tmp);
        tmp = std::nullopt;
    }

    new Instruction(
        instr++,
        Instruction::Opcode::Return, 
        operands,
        {}, 
        Metadata(span.begin.file, span.begin.line), 
        pInsert);
}

void Codegen::visit(Rune& node) {

}

void Codegen::visit(BoolLiteral& node) {
    tmp = Operand::get_imm(ValueType::Int8, node.get_value());
}

void Codegen::visit(IntegerLiteral& node) {
    tmp = Operand::get_imm(
        lower_type(node.get_type()), node.get_value());
}

void Codegen::visit(FloatLiteral& node) {
    tmp = Operand::get_fp(lower_type(node.get_type()), node.get_value());
}

void Codegen::visit(CharLiteral& node) {
    tmp = Operand::get_imm(ValueType::Int8, node.get_value());
}

void Codegen::visit(StringLiteral& node) {
    tmp = Operand::get_string(node.get_value().data());
}

void Codegen::visit(NullLiteral& node) {
    tmp = Operand::get_imm(ValueType::Pointer, 0);
}

void Codegen::visit(BinaryExpr& node) {
    
}

void Codegen::visit(UnaryExpr& node) {

}

void Codegen::visit(CastExpr& node) {

}

void Codegen::visit(ParenExpr& node) {

}

void Codegen::visit(SizeofExpr& node) {

}

void Codegen::visit(SubscriptExpr& node) {

}

void Codegen::visit(ReferenceExpr& node) {

}

void Codegen::visit(MemberExpr& node) {

}

void Codegen::visit(CallExpr& node) {

}

void Codegen::visit(RuneExpr& node) {

}
