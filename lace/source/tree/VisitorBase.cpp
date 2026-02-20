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
    m_scope = node.get_scope();

    for (Defn* defn : node.get_defns())
        defn->accept(*this);
}

void VisitorBase::visit(AliasDefn& node) {

}

void VisitorBase::visit(EnumDefn& node) {
    for (VariantDefn* variant : node.get_variants())
        variant->accept(*this);
}

void VisitorBase::visit(FieldDefn& node) {

}

void VisitorBase::visit(FunctionDefn& node) {
    m_scope = node.get_scope();

    if (node.has_body())
        node.get_body()->accept(*this);

    m_scope = m_scope->getParent();
}

void VisitorBase::visit(LoadDefn& node) {

}

void VisitorBase::visit(ParameterDefn& node) {

}

void VisitorBase::visit(StructDefn& node) {
    for (FieldDefn* field : node.get_fields())
        field->accept(*this);
}

void VisitorBase::visit(VariableDefn& node) {
    if (node.has_init())
        node.get_init()->accept(*this);
}

void VisitorBase::visit(VariantDefn& node) {

}

void VisitorBase::visit(AdapterStmt& node) {
    switch (node.get_flavor()) {
        case AdapterStmt::Definitive:
            node.get_defn()->accept(*this);
            break;
        case AdapterStmt::Expressive:
            node.get_expr()->accept(*this);
            break;
    }
}

void VisitorBase::visit(BlockStmt& node) {
    m_scope = node.get_scope();

    for (Stmt* stmt : node.get_stmts())
        stmt->accept(*this);

    m_scope = m_scope->getParent();
}

void VisitorBase::visit(IfStmt& node) {
    node.get_cond()->accept(*this);
    node.get_then()->accept(*this);

    if (node.has_else())
        node.get_else()->accept(*this);
}

void VisitorBase::visit(RestartStmt& node) {

}

void VisitorBase::visit(RetStmt& node) {
    if (node.has_expr())
        node.get_expr()->accept(*this);
}

void VisitorBase::visit(StopStmt& node) {

}

void VisitorBase::visit(UntilStmt& node) {
    node.get_cond()->accept(*this);

    if (node.has_body())
        node.get_body()->accept(*this);
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
    node.get_lhs()->accept(*this);
    node.get_rhs()->accept(*this);
}

void VisitorBase::visit(UnaryOp& node) {
    node.get_expr()->accept(*this);
}

void VisitorBase::visit(AccessExpr& node) {
    node.get_base()->accept(*this);
}

void VisitorBase::visit(CallExpr& node) {
    node.get_callee()->accept(*this);

    for (Expr* arg : node.get_args())
        arg->accept(*this);
}

void VisitorBase::visit(CastExpr& node) {
    node.get_expr()->accept(*this);
}

void VisitorBase::visit(ParenExpr& node) {
    node.get_expr()->accept(*this);
}

void VisitorBase::visit(RefExpr& node) {

}

void VisitorBase::visit(SizeofExpr& node) {

}

void VisitorBase::visit(SubscriptExpr& node) {
    node.get_base()->accept(*this);
    node.get_index()->accept(*this);
}

void VisitorBase::visit(StructInitExpr& node) {

}

Result VisitorBase::resolveType(const QualType& type) const {
    switch (type->getClass()) {
        case Type::Class::Array: {
            auto array_type = dynamic_cast<const ArrayType*>(type.getType());
            assert(array_type);

            return resolveType(array_type->element());
        }

        case Type::Class::Deferred: {
            auto deferred_type = dynamic_cast<const DeferredType*>(type.getType());
            assert(deferred_type);

            NamedDefn* named_defn = m_scope->get(deferred_type->name()); 
            if (!named_defn)
                return false;

            TypeDefn* type_defn = dynamic_cast<TypeDefn*>(named_defn);
            if (!type_defn)
                return false;

            type.setType(type_defn->get_type());
            return true;
        }

        case Type::Class::Enum: {
            auto enum_type = dynamic_cast<const EnumType*>(type.getType());
            assert(enum_type);

            return resolveType(enum_type->underlying());
        }

        case Type::Class::Function: {
            auto func_type = dynamic_cast<const FunctionType*>(type.getType());
            assert(func_type);

            if (!resolveType(func_type->result()))
                return false;

            for (const QualType& param : func_type->params()) {
                if (!resolveType(param))
                    return false;
            }

            return true;
        }

        case Type::Class::Pointer: {
            auto ptr_type = dynamic_cast<const PointerType*>(type.getType());
            assert(ptr_type);
            
            return resolveType(ptr_type->pointee());
        }

        default:
            return true;
    }
}
