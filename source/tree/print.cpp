#include "tree/decl.hpp"
#include "tree/expr.hpp"
#include "tree/root.hpp"
#include "tree/rune.hpp"
#include "tree/stmt.hpp"
#include "types/input_file.hpp"

#include <cstring>
#include <ostream>

using namespace stm;

static u32  gIndent = 0;
static bool gLastChild = false;
static bool gPipe[16] = {};

static void print_indent(std::ostream& os) {
    os << std::string(gIndent * 2, ' ');
}

static void print_piping(std::ostream& os) {
    for (u32 idx = 1; idx < gIndent; ++idx)
        os << (gPipe[idx] ? "│ " : "  ");

    if (gLastChild)
        os << "╰─";
    else
        os << "├─";
}

static std::string span_string(const Span& span) {
    return '<' + std::to_string(span.begin.line) + ':' + 
        std::to_string(span.begin.column) + "/" + 
        std::to_string(span.end.line) + ':' + 
        std::to_string(span.end.column) + '>';
}

void Root::print(std::ostream& os) const {
    std::memset(gPipe, false, 16);
    
    os << "Root <" << file.absolute() << ">\n";

    gPipe[++gIndent] = true;

    for (u32 idx = 0, e = decls.size(); idx != e; ++idx) {
        gLastChild = idx + 1 == e;
        gPipe[gIndent] = !gLastChild;
        decls[idx]->print(os);
    }

    os << '\n';
}

void FunctionDecl::print(std::ostream& os) const {
    print_piping(os);
    os << "Function " << span_string(span) << ' ' << name << " '" << 
        pType->to_string() << "'\n";

    gPipe[++gIndent] = true;
    for (auto& param : params)
        param->print(os);

    gLastChild = true;
    gPipe[gIndent] = false;
    pBody->print(os);
    gLastChild = false;
    gIndent--;
}

void ParameterDecl::print(std::ostream& os) const {
    print_piping(os);
    os << "Parameter " << span_string(span) << ' ' << name << " '" << 
        pType->to_string() << "'\n";
}

void VariableDecl::print(std::ostream& os) const {
    print_piping(os);
    os << "Variable " << span_string(span) << ' ' << name << " '" << 
        pType->to_string() << "'\n";

    if (has_init()) {
        gIndent++;
        gLastChild = true;
        pInit->print(os);
        gLastChild = false;
        gIndent--;
    }
}

void FieldDecl::print(std::ostream& os) const {
    print_piping(os);
    os << "Field " << span_string(span) << ' ' << name << " '" <<
        m_type->to_string() << "'\n";
}

void StructDecl::print(std::ostream& os) const {
    print_piping(os);
    os << "Structure " << span_string(span) << ' ' << name << " \n";

    gPipe[++gIndent] = true;

    for (u32 idx = 0, e = m_fields.size(); idx != e; ++idx) {
        gLastChild = idx + 1 == e;
        gPipe[gIndent] = !gLastChild;
        m_fields[idx]->print(os);
    }

    gIndent--;
}

void EnumValueDecl::print(std::ostream& os) const {
    print_indent(os);
    os << "Value " << span_string(span) << ' ' << name << ' ' << 
        std::to_string(m_value) << '\n';
}

void EnumDecl::print(std::ostream& os) const {
    print_piping(os);
    os << "Enum " << span_string(span) << ' ' << name << '\n';
    
    gPipe[++gIndent] = true;

    for (u32 idx = 0, e = m_values.size(); idx != e; ++idx) {
        gLastChild = idx + 1 == e;
        gPipe[gIndent] = !gLastChild;
        m_values[idx]->print(os);
    }

    gIndent--;
}

void BlockStmt::print(std::ostream& os) const {
    print_piping(os);
    os << "Block " << span_string(span) << '\n';

    if (is_empty()) return;

    gPipe[++gIndent] = true;
    for (u32 idx = 0, e = stmts.size(); idx != e; ++idx) {
        gLastChild = idx + 1 == e;
        gPipe[gIndent] = !gLastChild;
        stmts[idx]->print(os);
    }

    gIndent--;
}

void BreakStmt::print(std::ostream& os) const {
    print_piping(os);
    os << "Break " << span_string(span) << '\n';
}

void ContinueStmt::print(std::ostream& os) const {
    print_piping(os);
    os << "Continue " << span_string(span) << '\n';
}

void DeclStmt::print(std::ostream& os) const {
    pDecl->print(os);
}

void IfStmt::print(std::ostream& os) const {
    print_piping(os);
    os << "If " << span_string(span) << '\n';

    gPipe[++gIndent] = true;
    pCond->print(os);

    if (has_else()) {
        get_then()->print(os);
        gLastChild = true;
        gPipe[gIndent] = false;
        get_else()->print(os);
    } else {
        gLastChild = true;
        gPipe[gIndent] = false;
        get_then()->print(os);
    }

    gIndent--;
}

void WhileStmt::print(std::ostream& os) const {
    print_piping(os);
    os << "While " << span_string(span) << '\n';

    gPipe[++gIndent] = true;
    gLastChild = false;
    pCond->print(os);

    gPipe[gIndent] = false;
    gLastChild = true;
    pBody->print(os);
    gIndent--;
}

void RetStmt::print(std::ostream& os) const {
    print_piping(os);
    os << "Return " << span_string(span) << '\n';

    if (has_expr()) {
        gIndent++;
        gLastChild = true;
        pExpr->print(os);
        gIndent--;
    }
}

void Rune::print(std::ostream& os) const {
    
}

void BoolLiteral::print(std::ostream& os) const {
    print_piping(os);
    os << "Boolean " << span_string(span) << ' ' << std::to_string(value) 
        << " '" << pType->to_string() << "'\n"; 
}

void IntegerLiteral::print(std::ostream& os) const {
    print_piping(os);
    os << "Integer "  << span_string(span) << ' ' << std::to_string(value) 
        << " '" << pType->to_string() << "'\n"; 
}

void FloatLiteral::print(std::ostream& os) const {
    print_piping(os);
    os << "Float " << span_string(span) << ' ' << std::to_string(value) 
        << " '" << pType->to_string() << "'\n"; 
}

void CharLiteral::print(std::ostream& os) const {
    print_piping(os);
    os << "Character " << span_string(span) << ' ' << value << " '" 
        << pType->to_string() << "'\n"; 
}

void StringLiteral::print(std::ostream& os) const {
    print_piping(os);
    os << "String " << span_string(span) << " \"" << value << "\" '" 
        << pType->to_string() << "'\n"; 
}

void NullLiteral::print(std::ostream& os) const {
    print_piping(os);
    os << "Nil " << span_string(span) << " '" << pType->to_string() << "'\n";
}

void BinaryExpr::print(std::ostream& os) const {
    print_piping(os);
    os << "Binary " << span_string(span) << ' ';

    switch (op) {
    case Operator::Add: 
        os << "+"; 
        break;
    case Operator::Add_Assign: 
        os << "+="; 
        break;
    case Operator::Sub: 
        os << "-"; 
        break;
    case Operator::Sub_Assign: 
        os << "-="; 
        break;
    case Operator::Mul: 
        os << "*"; 
        break;
    case Operator::Mul_Assign: 
        os << "*="; 
        break;
    case Operator::Div: 
        os << "/"; 
        break;
    case Operator::Div_Assign: 
        os << "/="; 
        break;
    case Operator::Mod: 
        os << "%"; 
        break;
    case Operator::Mod_Assign: 
        os << "%="; 
        break;
    case Operator::Less_Than: 
        os << "<"; 
        break;
    case Operator::Less_Than_Equals: 
        os << "<="; 
        break;
    case Operator::Greater_Than: 
        os << ">"; 
        break;
    case Operator::Greater_Than_Equals: 
        os << ">="; 
        break;
    case Operator::Bitwise_And: 
        os << "&"; 
        break;
    case Operator::Bitwise_And_Assign: 
        os << "&="; 
        break;
    case Operator::Bitwise_Or: 
        os << "|"; 
        break;
    case Operator::Bitwise_Or_Assign: 
        os << "|="; 
        break;
    case Operator::Bitwise_Xor: 
        os << "^"; 
        break;
    case Operator::Bitwise_Xor_Assign: 
        os << "^="; 
        break;
    case Operator::Logical_And: 
        os << "&&"; 
        break;
    case Operator::Logical_Or: 
        os << "||"; 
        break;
    case Operator::Left_Shift: 
        os << "<<"; 
        break;
    case Operator::Left_Shift_Assign: 
        os << "<<="; 
        break;
    case Operator::Right_Shift: 
        os << ">>"; 
        break;
    case Operator::Right_Shift_Assign: 
        os << ">>="; 
        break;
    case Operator::Assign: 
        os << "="; 
        break;
    case Operator::Equals: 
        os << "=="; 
        break;
    case Operator::Not_Equals: 
        os << "!="; 
        break;
    default: 
        break;
    }

    os << " '" << pType->to_string() << "'\n";

    gLastChild = false;
    gPipe[++gIndent] = true;
    pLeft->print(os);

    gLastChild = true;
    gPipe[gIndent] = false;
    pRight->print(os);
    gIndent--;
}

void UnaryExpr::print(std::ostream& os) const {
    print_piping(os);
    os << "Unary " << span_string(span) << ' ';

    switch (op) {
        case Operator::Increment: os << "++"; break;
        case Operator::Decrement: os << "--"; break;
        case Operator::Dereference: os << "*"; break;
        case Operator::Address_Of: os << "&"; break;
        case Operator::Logical_Not: os << "!"; break;
        case Operator::Bitwise_Not: os << "~"; break;
        default: break;
    }

    os << " '" << pType->to_string() << "'\n";

    gIndent++;
    pExpr->print(os);
    gIndent--;
}

void CastExpr::print(std::ostream& os) const {
    print_piping(os);
    os << "Cast " << span_string(span) << " '" << pType->to_string() << "'\n";

    gIndent++;
    pExpr->print(os);
    gIndent--;
}

void ParenExpr::print(std::ostream& os) const {
    print_piping(os);
    os << "Parentheses " << span_string(span) << ' ' << pType->to_string() << 
        '\n';

    gIndent++;
    pExpr->print(os);
    gIndent--;
}

void SizeofExpr::print(std::ostream& os) const {
    print_piping(os);
    os << "Sizeof " << span_string(span) << " '" << pTarget->to_string() << 
        "' " << pType->to_string() << '\n';
}

void SubscriptExpr::print(std::ostream& os) const {
    print_piping(os);
    os << "Subscript " << span_string(span) << '\n';

    gIndent++;
    gLastChild = false;
    pBase->print(os);
    gLastChild = true;
    pIndex->print(os);
    gIndent--;
}

void ReferenceExpr::print(std::ostream& os) const {
    print_piping(os);
    os << "Reference " << span_string(span) << ' ' << name << " '" << 
        pType->to_string() << "'\n";
}

void MemberExpr::print(std::ostream& os) const {
    print_piping(os);
    os << "Member " << span_string(span) << ' ' << name << " '" << 
        pType->to_string() << "'\n";

    gIndent++;
    pBase->print(os);
    gIndent--;
}

void CallExpr::print(std::ostream& os) const {
    print_piping(os);
    os << "Call " << span_string(span) << ' ' << name << " '" << 
        pType->to_string() << "'\n";

    gIndent++;
    for (auto& arg : args) arg->print(os);
    gIndent--;
}

void RuneExpr::print(std::ostream& os) const {
    
}
