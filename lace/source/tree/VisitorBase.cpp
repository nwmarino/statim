//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Rib.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Stmt.h"
#include "lace/tree/VisitorBase.h"

using namespace lace;

void VisitorBase::visit(Rib& rib) {
    m_rib = &rib;

    for (Defn* defn : rib.defns())
        defn->accept(*this);
}

void VisitorBase::visit(AliasDefn& node) {

}

void VisitorBase::visit(EnumDefn& node) {
    for (VariantDefn* variant : node.variants())
        variant->accept(*this);
}

void VisitorBase::visit(FieldDefn& node) {

}

void VisitorBase::visit(FunctionDefn& node) {
    if (node.has_receiver())
        node.receiver()->accept(*this);

    for (ParameterDefn* param : node.params())
        param->accept(*this);

    if (node.has_body())
        node.body()->accept(*this);
}

void VisitorBase::visit(ParameterDefn& node) {

}

void VisitorBase::visit(StructDefn& node) {
    for (FieldDefn* field : node.fields())
        field->accept(*this);
}

void VisitorBase::visit(VariableDefn& node) {
    if (node.has_init())
        node.init()->accept(*this);
}

void VisitorBase::visit(VariantDefn& node) {

}

void VisitorBase::visit(AdapterStmt& node) {
    switch (node.kind()) 
    {
    case AdapterStmt::Kind::Definitive:
        node.defn()->accept(*this);
        break;
    case AdapterStmt::Kind::Expressive:
        node.expr()->accept(*this);
        break;
    }
}

void VisitorBase::visit(BlockStmt& node) {
    for (Stmt* stmt : node.stmts())
        stmt->accept(*this);
}

void VisitorBase::visit(IfStmt& node) {
    node.condition()->accept(*this);
    node.then_body()->accept(*this);

    if (node.has_else())
        node.else_body()->accept(*this);
}

void VisitorBase::visit(RestartStmt& node) {

}

void VisitorBase::visit(RetStmt& node) {
    if (node.has_expr())
        node.expr()->accept(*this);
}

void VisitorBase::visit(StopStmt& node) {

}

void VisitorBase::visit(UntilStmt& node) {
    node.condition()->accept(*this);

    if (node.has_body())
        node.body()->accept(*this);
}

void VisitorBase::visit(RuneStmt& node) {

}

void VisitorBase::visit(BoolLiteral& node) {

}

void VisitorBase::visit(CharLiteral& node) {

}

void VisitorBase::visit(IntegerLiteral& node) {

}

void VisitorBase::visit(FloatLiteral& node) {

}

void VisitorBase::visit(NullLiteral& node) {

}

void VisitorBase::visit(StringLiteral& node) {

}

void VisitorBase::visit(BinaryOp& node) {
    node.lhs()->accept(*this);
    node.rhs()->accept(*this);
}

void VisitorBase::visit(UnaryOp& node) {
    node.expr()->accept(*this);
}

void VisitorBase::visit(AccessExpr& node) {
    node.base()->accept(*this);
}

void VisitorBase::visit(CallExpr& node) {
    node.callee()->accept(*this);

    for (Expr* arg : node.args())
        arg->accept(*this);
}

void VisitorBase::visit(CastExpr& node) {
    node.expr()->accept(*this);
}

void VisitorBase::visit(ParenExpr& node) {
    node.expr()->accept(*this);
}

void VisitorBase::visit(RefExpr& node) {

}

void VisitorBase::visit(SizeofExpr& node) {

}

void VisitorBase::visit(StructInitExpr& node) {
    for (auto& [field, expr] : node.fields())
        expr->accept(*this);
}

void VisitorBase::visit(SubscriptExpr& node) {
    node.base()->accept(*this);
    node.index()->accept(*this);
}
