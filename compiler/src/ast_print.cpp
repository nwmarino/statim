#include "ast.hpp"

using namespace stm;

static u32 gIndent = 0;

static std::string indent() {
    return std::string(gIndent * 2, ' ');
}

static std::string span_string(const Span& span) {
    return '<' + span.begin.file.filename() + ':' + std::to_string(span.begin.line) + ':' 
    + std::to_string(span.begin.column) + " - " + std::to_string(span.end.line) + ':' 
    + std::to_string(span.end.column) + '>';
}

void FunctionDecl::print(std::ostream& os) const {
    os << "Function " << span_string(span) << " '" << name << "' " << pType->to_string() << '\n';

    gIndent++;
    for (auto& param : params)
        param->print(os);

    pBody->print(os);
    gIndent--;
}

void ParameterDecl::print(std::ostream& os) const {
    os << "Parameter " << span_string(span) << " '" << name << "' " << pType->to_string() << '\n';
}

void VariableDecl::print(std::ostream& os) const {
    os << "Variable " << span_string(span) << " '" << name << "' " << pType->to_string() << '\n';

    if (has_init()) {
        gIndent++;
        pInit->print(os);
        gIndent--;
    }
}

void BlockStmt::print(std::ostream& os) const {
    os << "Block " << span_string(span) << '\n';

    gIndent++;
    for (auto& stmt : stmts) stmt->print(os);
    gIndent--;
}

void BreakStmt::print(std::ostream& os) const {
    os << "Break " << span_string(span) << '\n';
}

void ContinueStmt::print(std::ostream& os) const {
    os << "Continue " << span_string(span) << '\n';
}

void DeclStmt::print(std::ostream& os) const {
    pDecl->print(os);
}

void IfStmt::print(std::ostream& os) const {
    os << "If " << span_string(span) << '\n';

    gIndent++;
    pCond->print(os);
    pThen->print(os);

    if (has_else())
        pElse->print(os);

    gIndent--;
}

void WhileStmt::print(std::ostream& os) const {
    os << "While " << span_string(span) << '\n';

    gIndent++;
    pCond->print(os);
    pBody->print(os);
    gIndent--;
}

void RetStmt::print(std::ostream& os) const {
    os << "Return " << span_string(span) << '\n';

    if (has_expr()) {
        gIndent++;
        pExpr->print(os);
        gIndent--;
    }
}

void Rune::print(std::ostream& os) const {
    
}

void BoolLiteral::print(std::ostream& os) const {
    os << "Boolean " << span_string(span) << " '" << std::to_string(value) << "' " << pType->to_string() << '\n'; 
}

void IntegerLiteral::print(std::ostream& os) const {
    os << "Integer "  << span_string(span) << " '" << std::to_string(value) << "' " << pType->to_string() << '\n'; 
}

void FloatLiteral::print(std::ostream& os) const {
    os << "Float " << span_string(span) << " '" << std::to_string(value) << "' " << pType->to_string() << '\n'; 
}

void CharLiteral::print(std::ostream& os) const {
    os << "Character " << span_string(span) << " '" << value << "' " << pType->to_string() << '\n'; 
}

void StringLiteral::print(std::ostream& os) const {
    os << "String " << span_string(span) << " \"" << value << "\" " << pType->to_string() << '\n'; 
}

void NilLiteral::print(std::ostream& os) const {
    os << "Nil " << span_string(span) << pType->to_string() << '\n';
}

void BinaryExpr::print(std::ostream& os) const {
    os << "BinaryOp " << span_string(span) << ' ';

    switch (op) {
        case Operator::Add: os << "+"; break;
        case Operator::Add_Assign: os << "+="; break;
        case Operator::Sub: os << "-"; break;
        case Operator::Sub_Assign: os << "-="; break;
        case Operator::Mul: os << "*"; break;
        case Operator::Mul_Assign: os << "*="; break;
        case Operator::Div: os << "/"; break;
        case Operator::Div_Assign: os << "/="; break;
        case Operator::Mod: os << "%"; break;
        case Operator::Mod_Assign: os << "%="; break;
        case Operator::Less_Than: os << "<"; break;
        case Operator::Less_Than_Equals: os << "<="; break;
        case Operator::Greater_Than: os << ">"; break;
        case Operator::Greater_Than_Equals: os << ">="; break;
        case Operator::Bitwise_And: os << "&"; break;
        case Operator::Bitwise_And_Assign: os << "&="; break;
        case Operator::Bitwise_Or: os << "|"; break;
        case Operator::Bitwise_Or_Assign: os << "|="; break;
        case Operator::Bitwise_Xor: os << "^"; break;
        case Operator::Bitwise_Xor_Assign: os << "^="; break;
        case Operator::Logical_And: os << "&&"; break;
        case Operator::Logical_Or: os << "||"; break;
        case Operator::Left_Shift: os << "<<"; break;
        case Operator::Left_Shift_Assign: os << "<<="; break;
        case Operator::Right_Shift: os << ">>"; break;
        case Operator::Right_Shift_Assign: os << ">>="; break;
    }

    os << ' ' << pType->to_string() << '\n';

    gIndent++;
    pLeft->print(os);
    pRight->print(os);
    gIndent--;
}

void UnaryExpr::print(std::ostream& os) const {
    os << "UnaryOp " << span_string(span) << ' ';

    switch (op) {
        case Operator::Increment: os << "++"; break;
        case Operator::Decrement: os << "--"; break;
        case Operator::Dereference: os << "*"; break;
        case Operator::Address_Of: os << "&"; break;
        case Operator::Logical_Not: os << "!"; break;
        case Operator::Bitwise_Not: os << "~"; break;
    }

    os << ' ' << pType->to_string() << '\n';

    gIndent++;
    pExpr->print(os);
    gIndent--;
}

void CastExpr::print(std::ostream& os) const {
    os << "Cast " << span_string(span) << ' ' << pType->to_string() << '\n';

    gIndent++;
    pExpr->print(os);
    gIndent--;
}

void ParenExpr::print(std::ostream& os) const {
    os << "Paren " << span_string(span) << ' ' << pType->to_string() << '\n';

    gIndent++;
    pExpr->print(os);
    gIndent--;
}

void SizeofExpr::print(std::ostream& os) const {
    os << "Sizeof " << span_string(span) << " '" << pTarget->to_string() << "' " << 
        pType->to_string() << '\n';
}

void SubscriptExpr::print(std::ostream& os) const {
    os << "Subscript " << span_string(span) << '\n';

    gIndent++;
    pBase->print(os);
    pIndex->print(os);
    gIndent--;
}

void ReferenceExpr::print(std::ostream& os) const {
    os << "Reference " << span_string(span) << " '" << ref << "' " << pType->to_string() << '\n';
}

void MemberExpr::print(std::ostream& os) const {
    os << "Access " << span_string(span) << " '" << ref << "' " << pType->to_string() << '\n';

    gIndent++;
    pBase->print(os);
    gIndent--;
}

void CallExpr::print(std::ostream& os) const {
    os << "Call " << span_string(span) << " '" << ref << "' " << pType->to_string() << '\n';

    gIndent++;
    for (auto& arg : args) arg->print(os);
    gIndent--;
}

void RuneExpr::print(std::ostream& os) const {
    
}
