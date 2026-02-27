//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Printer.h"
#include "lace/tree/Rib.h"
#include "lace/tree/Stmt.h"
#include "lace/tree/VisitorBase.h"
#include "lace/types/SourceLocation.h"
#include "lace/types/SourceSpan.h"

#include <format>
#include <ostream>

using namespace lace;

Printer::Printer(Context& context, std::ostream& out) 
  : VisitorBase(context), m_out(out) {}

void Printer::visit(Rib& rib) {
    m_out << std::format("Rib {} \"{}\"\n", rib.name(), rib.path());

    ++m_indent;

    VisitorBase::visit(rib);

    --m_indent;
}

void Printer::visit(AliasDefn& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    const AliasType* type = dynamic_cast<const AliasType*>(node.type());
    assert(type);

    m_out << std::format("Alias <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.name(),
        type->aliased()->string()
    );
}

void Printer::visit(EnumDefn& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Enum <{}:{}, {}:{}> {}\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.name()
    );

    ++m_indent;

    for (VariantDefn* variant : node.variants())
        variant->accept(*this);

    --m_indent;
}

void Printer::visit(FieldDefn& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Field <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.name(),
        node.type()->string()
    );
}

void Printer::visit(FunctionDefn& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Function <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.name(),
        node.type()->string()
    );

    ++m_indent;

    for (ParameterDefn* param : node.params())
        param->accept(*this);

    if (node.has_body())
        node.body()->accept(*this);

    --m_indent;
}

void Printer::visit(ParameterDefn& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;
    
    m_out << std::format("Parameter <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.name(),
        node.type()->string()
    );
}

void Printer::visit(StructDefn& node) {
    print_indent();
    
    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Struct <{}:{}, {}:{}> {}\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.name()
    );

    ++m_indent;

    for (FieldDefn* field : node.fields())
        field->accept(*this);

    --m_indent;
}

void Printer::visit(UseDefn& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.end;

    m_out << std::format("Use <{}:{}, {}:{}> \"{}\"\n", 
        start.line, 
        start.col, 
        end.line, 
        end.col,
        node.path()
    );
}

void Printer::visit(VariableDefn& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Variable <{}:{}, {}:{}> {} '{}'\n", 
        start.line, 
        start.col, 
        end.line, 
        end.col, 
        node.name(), 
        node.type()->string()
    );

    if (node.has_init()) {
        ++m_indent;
        node.init()->accept(*this);
        --m_indent;
    }
}

void Printer::visit(VariantDefn& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Variant <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.name(),
        node.type()->string()
    );
}

void Printer::visit(AdapterStmt& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Adapter <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(BlockStmt& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Block <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(IfStmt& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("If <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(RestartStmt& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Restart <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col
    );
}

void Printer::visit(RetStmt& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Ret <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(StopStmt& node) {
        print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Stop <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col
    );
}

void Printer::visit(UntilStmt& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Until <{}:{}, {}:{}>\n",
        start.line,
        start.col,
        end.line,
        end.col
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(BoolLiteral& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Bool <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_value(),
        node.type()->string()
    );
}

void Printer::visit(CharLiteral& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Char <{}:{}, {}:{}> '{}' '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_value(),
        node.type()->string()
    );
}

void Printer::visit(IntegerLiteral& node) {
        print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Integer <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_value(),
        node.type()->string()
    );
}

void Printer::visit(FloatLiteral& node) {
        print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Float <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.get_value(),
        node.type()->string()
    );
}

void Printer::visit(NullLiteral& node) {
        print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Null <{}:{}, {}:{}> '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.type()->string()
    );
}

void Printer::visit(StringLiteral& node) {
        print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("String <{}:{}, {}:{}> \"{}\" '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.value(),
        node.type()->string()
    );
}

void Printer::visit(BinaryOp& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Binary <{}:{}, {}:{}> '{}' ",
        start.line,
        start.col,
        end.line,
        end.col,
        node.type()->string()
    );

    switch (node.op()) 
    {
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

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(UnaryOp& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Unary <{}:{}, {}:{}> '{}' ",
        start.line,
        start.col,
        end.line,
        end.col,
        node.type()->string()
    );

    switch (node.op()) 
    {
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

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(AccessExpr& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Access <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.name(),
        node.type()->string()
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(CallExpr& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Call <{}:{}, {}:{}> '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.type()->string()
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(CastExpr& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Cast <{}:{}, {}:{}> '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.type()->string()
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(FieldInitExpr& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("FieldInit <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.name(),
        node.type()->string()
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(ParenExpr& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Paren <{}:{}, {}:{}> '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.type()->string()
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(RefExpr& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Ref <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.name(),
        node.type()->string()
    );

    if (node.has_spec()) {
        m_indent++;
        print_indent();
        m_out << std::format("Specifier \"{}\"\n", node.spec());
        m_indent--;
    }
}

void Printer::visit(SizeofExpr& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Sizeof <{}:{}, {}:{}> {} '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.target()->string(),
        node.type()->string()
    );
}

void Printer::visit(StructInitExpr& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.end;

    m_out << std::format("StructInit <{}:{}, {}:{}> '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.type()->string()
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}

void Printer::visit(SubscriptExpr& node) {
    print_indent();

    const SourceSpan span = node.span();
    const SourceLocation start = span.start, end = span.start;

    m_out << std::format("Subscript <{}:{}, {}:{}> '{}'\n",
        start.line,
        start.col,
        end.line,
        end.col,
        node.type()->string()
    );

    ++m_indent;

    VisitorBase::visit(node);

    --m_indent;
}
