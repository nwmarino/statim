//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Stmt.h"
#include "lace/tree/VisitorBase.h"

using namespace lace;

void VisitorBase::visit(AST& node) {
    m_ast = &node;
    m_scope = node.scope();

    for (Defn* defn : node.defns())
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
    m_scope = node.scope();

    if (node.has_body())
        node.body()->accept(*this);

    m_scope = m_scope->parent();
}

void VisitorBase::visit(LoadDefn& node) {

}

void VisitorBase::visit(ParameterDefn& node) {

}

void VisitorBase::visit(SpaceDefn& node) {
    m_namespaces.push_back(&node);

    for (NamedDefn* defn : node.defns()) {
        // Only pass over definitions defined in the same file.
        if (defn->origin() == node.origin())
            defn->accept(*this);
    }

    m_namespaces.pop_back();
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
    m_scope = node.scope();

    for (Stmt* stmt : node.stmts())
        stmt->accept(*this);

    m_scope = m_scope->parent();
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

Type* VisitorBase::resolve_type(Type* type) const {
    if (auto deferred = dynamic_cast<DeferredType*>(type)) {
        NamedDefn* named_defn = m_scope->get(deferred->name()); 
        if (!named_defn)
            return nullptr;

        TypeDefn* type_defn = dynamic_cast<TypeDefn*>(named_defn);
        if (!type_defn)
            return nullptr;

        return type_defn->type();
    } else if (auto enumeration = dynamic_cast<EnumType*>(type)) {
        Type* underlying = resolve_type(enumeration->underlying());
        if (underlying != enumeration->underlying())
            enumeration->set_underlying(underlying);

        return enumeration;
    } else if (auto func = dynamic_cast<FunctionType*>(type)) {
        Type* result = resolve_type(func->result());
        if (!result)
            return nullptr;

        std::vector<Type*> params = {};
        params.reserve(func->num_params());

        for (Type* param : func->params()) {
            Type* res = resolve_type(param);
            if (!res)
                return nullptr;

            params.push_back(res);
        }

        return FunctionType::get(*m_ast, result, params);
    } else if (auto ptr = dynamic_cast<PointerType*>(type)) {
        Type* pointee = resolve_type(ptr->pointee());
        if (pointee != ptr->pointee())
            ptr->set_pointee(pointee);

        return ptr;
    }

    return type;
}
