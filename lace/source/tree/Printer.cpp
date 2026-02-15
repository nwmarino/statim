//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/tree/Defn.hpp"
#include "lace/tree/Expr.hpp"
#include "lace/tree/Printer.hpp"
#include "lace/tree/Stmt.hpp"
#include "lace/tree/VisitorBase.hpp"

#include <format>
#include <ostream>

using namespace lace;

Printer::Printer(Options& options, std::ostream& out) : VisitorBase(options), m_out(out) {}

void Printer::visit(AST& node) {
    m_out << std::format("AST \"{}\"\n", node.get_file());

    ++m_indent;

    VisitorBase::visit(node);
    
    --m_indent;
}

void Printer::visit(LoadDefn& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.end;

    m_out << std::format("Load <{}:{}, {}:{}> \"{}\"\n", 
        start.line, 
        start.col, 
        end.line, 
        end.col, 
        node.get_path());
}

void Printer::visit(VariableDefn& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Variable <{}:{}, {}:{}> {} '{}'\n", 
        start.line, 
        start.col, 
        end.line, 
        end.col, 
        node.get_name(), 
        node.get_type().to_string());

    if (node.has_init()) {
        ++m_indent;
        node.get_init()->accept(*this);
        --m_indent;
    }
}

void Printer::visit(ParameterDefn& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;
    
    m_out << std::format("Parameter <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_name(),
        node.get_type().to_string());
}

void Printer::visit(FunctionDefn& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Function <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_name(),
        node.get_type().to_string());

    ++m_indent;

    for (ParameterDefn* param : node.get_params())
        param->accept(*this);

    if (node.has_body())
        node.get_body()->accept(*this);

    --m_indent;
}

void Printer::visit(FieldDefn& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Field <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_name(),
        node.get_type().to_string());
}

void Printer::visit(VariantDefn& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Variant <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_name(),
        node.get_type().to_string());
}

void Printer::visit(AliasDefn& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    const AliasType* type = dynamic_cast<const AliasType*>(node.get_type());
    assert(type);

    m_out << std::format("Alias <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_name(),
        type->get_underlying().to_string());
}

void Printer::visit(StructDefn& node) {
    print_indent();
    
    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Struct <{}:{}, {}:{}> {}\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_name());

    ++m_indent;

    for (FieldDefn* field : node.get_fields())
        field->accept(*this);

    --m_indent;
}

void Printer::visit(EnumDefn& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Enum <{}:{}, {}:{}> {}\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_name());

    ++m_indent;

    for (VariantDefn* variant : node.get_variants())
        variant->accept(*this);

    --m_indent;
}

void Printer::visit(AdapterStmt& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Adapter <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col);

    ++m_indent;

    switch (node.get_flavor()) {
    case AdapterStmt::Definitive:
        node.get_defn()->accept(*this);
        break;

    case AdapterStmt::Expressive:
        node.get_expr()->accept(*this);
        break;
    }

    --m_indent;
}

void Printer::visit(BlockStmt& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Block <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col);

    ++m_indent;

    for (Stmt* stmt : node.get_stmts())
        stmt->accept(*this);

    --m_indent;
}

void Printer::visit(IfStmt& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("If <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col);

    ++m_indent;

    node.get_cond()->accept(*this);
    node.get_then()->accept(*this);

    if (node.has_else())
        node.get_else()->accept(*this);

    --m_indent;
}

void Printer::visit(RestartStmt& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Restart <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col);
}

void Printer::visit(RetStmt& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Ret <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col);

    if (node.has_expr()) {
        ++m_indent;
        node.get_expr()->accept(*this);
        --m_indent;
    }
}

void Printer::visit(StopStmt& node) {
        print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Stop <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col);
}

void Printer::visit(UntilStmt& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Until <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col);

    ++m_indent;

    node.get_cond()->accept(*this);

    if (node.has_body())
        node.get_body()->accept(*this);

    --m_indent;
}

void Printer::visit(BoolLiteral& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Bool <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_value(),
        node.get_type().to_string());
}

void Printer::visit(CharLiteral& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Char <{}:{}, {}:{}> '{}' '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_value(),
        node.get_type().to_string());
}

void Printer::visit(IntegerLiteral& node) {
        print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Integer <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_value(),
        node.get_type().to_string());
}

void Printer::visit(FloatLiteral& node) {
        print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Float <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_value(),
        node.get_type().to_string());
}

void Printer::visit(NullLiteral& node) {
        print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Null <{}:{}, {}:{}> '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_type().to_string());
}

void Printer::visit(StringLiteral& node) {
        print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("String <{}:{}, {}:{}> \"{}\" '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_value(),
        node.get_type().to_string());
}

void Printer::visit(BinaryOp& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Binary <{}:{}, {}:{}> '{}' ",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_type().to_string());

    switch (node.get_operator()) {
        case BinaryOp::Assign:
            m_out << "=";
            break;
        case BinaryOp::Add:
            m_out << "+";
            break;
        case BinaryOp::Sub:
            m_out << "-";
            break;
        case BinaryOp::Mul:
            m_out << "*";
            break;
        case BinaryOp::Div:
            m_out << "/";
            break;
        case BinaryOp::Mod:
            m_out << "%";
            break;
        case BinaryOp::And:
            m_out << "&";
            break;
        case BinaryOp::Or:
            m_out << "|";
            break;
        case BinaryOp::Xor:
            m_out << "^";
            break;
        case BinaryOp::LShift:
            m_out << "<<";
            break;
        case BinaryOp::RShift:
            m_out << ">>";
            break;
        case BinaryOp::LogicAnd:
            m_out << "&&";
            break;
        case BinaryOp::LogicOr:
            m_out << "||";
            break;
        case BinaryOp::Eq:
            m_out << "=";
            break;
        case BinaryOp::NEq:
            m_out << "!=";
            break;
        case BinaryOp::Lt:
            m_out << "<";
            break;
        case BinaryOp::LtEq:
            m_out << "<=";
            break;
        case BinaryOp::Gt:
            m_out << ">";
            break;
        case BinaryOp::GtEq:
            m_out << ">=";
            break;
        default:
            assert(false && "unknown binary operator!");
    }

    m_out << '\n';

    ++m_indent;
    node.get_lhs()->accept(*this);
    node.get_rhs()->accept(*this);
    --m_indent;
}

void Printer::visit(UnaryOp& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Unary <{}:{}, {}:{}> '{}' ",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_type().to_string());

    switch (node.get_operator()) {
        case UnaryOp::Negate:
            m_out << "-";
            break;
        case UnaryOp::Not:
            m_out << "~";
            break;
        case UnaryOp::LogicNot:
            m_out << "!";
            break;
        case UnaryOp::AddressOf:
            m_out << "&";
            break;
        case UnaryOp::Dereference:
            m_out << "*";
            break;
        default:
            assert(false && "unknown unary operator!");
    }

    m_out << '\n';

    ++m_indent;
    node.get_expr()->accept(*this);
    --m_indent;
}

void Printer::visit(AccessExpr& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Access <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_name(),
        node.get_type().to_string());

    ++m_indent;
    node.get_base()->accept(*this);
    --m_indent;
}

void Printer::visit(CallExpr& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Call <{}:{}, {}:{}> '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_type().to_string());

    ++m_indent;

    node.get_callee()->accept(*this);
    for (Expr* arg : node.get_args())
        arg->accept(*this);

    --m_indent;
}

void Printer::visit(CastExpr& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Cast <{}:{}, {}:{}> '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_type().to_string());

    ++m_indent;
    node.get_expr()->accept(*this);
    --m_indent;
}

void Printer::visit(ParenExpr& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Paren <{}:{}, {}:{}> '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_type().to_string());

    ++m_indent;
    node.get_expr()->accept(*this);
    --m_indent;
}

void Printer::visit(RefExpr& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Ref <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_name(),
        node.get_type().to_string());
}

void Printer::visit(SizeofExpr& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Sizeof <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_target_type().to_string(),
        node.get_type().to_string());
}

void Printer::visit(SubscriptExpr& node) {
    print_indent();

    const SourceSpan span = node.get_span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Subscript <{}:{}, {}:{}> '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_type().to_string());

    ++m_indent;
    node.get_base()->accept(*this);
    node.get_index()->accept(*this);
    --m_indent;
}
