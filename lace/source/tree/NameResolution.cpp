//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/tree/NameResolution.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Rib.h"
#include "lace/tree/Stmt.h"
#include "lace/tree/Type.h"
#include "lace/tree/VisitorBase.h"

using namespace lace;

NameResolution::NameResolution(Context& context) : VisitorBase(context) {}

void NameResolution::visit(Rib& rib) {
    m_scope = rib.scope();

    VisitorBase::visit(rib);

    m_scope = m_scope->parent();
}

void NameResolution::visit(AliasDefn& node) {
    Type* type = resolve_type(node.type());
    if (!type) {
        log::error("unresolved alias type: '" + node.type()->string() + "'", 
            log::Span { m_rib->path(), node.span() });

        return;
    }

    node.set_type(type);

    VisitorBase::visit(node);
}

void NameResolution::visit(FieldDefn& node) {
    Type* type = resolve_type(node.type());
    if (!type) {
        log::error("unresolved field type: '" + node.type()->string() + "'", 
            log::Span { m_rib->path(), node.span() });

        return;
    }

    node.set_type(type);

    VisitorBase::visit(node);
}

void NameResolution::visit(FunctionDefn& node) {
    Type* type = resolve_type(node.type());
    if (!type) {
        log::error("unresolved function type: '" + node.type()->string() + "'", 
            log::Span { m_rib->path(), node.span() });

        return;
    }

    FunctionType* ft = dynamic_cast<FunctionType*>(type);
    assert(ft);

    node.set_type(ft);

    m_scope = node.scope();

    VisitorBase::visit(node);

    m_scope = m_scope->parent();
}

void NameResolution::visit(ParameterDefn& node) {
    Type* type = resolve_type(node.type());
    if (!type) {
        log::error("unresolved parameter type: '" + node.type()->string() + "'", 
            log::Span { m_rib->path(), node.span() });

        return;
    }

    node.set_type(type);

    VisitorBase::visit(node);
}

void NameResolution::visit(UseDefn& node) {
    Rib* rib = m_context.get_rib(node.path());
    if (!rib) {
        log::error("unresolved rib: '" + node.path() + "'",
            log::Span { m_rib->path(), node.span() });

        return;
    }

    node.set_target(rib);

    VisitorBase::visit(node);
}

void NameResolution::visit(VariableDefn& node) {
    Type* type = resolve_type(node.type());
    if (!type) {
        log::error("unresolved variable type: '" + node.type()->string() + "'", 
            log::Span { m_rib->path(), node.span() });

        return;
    }

    node.set_type(type);

    VisitorBase::visit(node);
}

void NameResolution::visit(BlockStmt& node) {
    m_scope = node.scope();

    VisitorBase::visit(node);

    m_scope = m_scope->parent();
}

void NameResolution::visit(AccessExpr& node) {
    VisitorBase::visit(node);  
}

void NameResolution::visit(CastExpr& node) {
    Type* type = resolve_type(node.type());
    if (!type) {
        log::error("unresolved cast type: '" + node.type()->string() + "'", 
            log::Span { m_rib->path(), node.span() });

        return;
    }

    node.set_type(type);

    VisitorBase::visit(node);
}

void NameResolution::visit(FieldInitExpr& node) {
    VisitorBase::visit(node);
}

void NameResolution::visit(RefExpr& node) {
    Scope* prev_scope = m_scope;

    if (node.has_spec()) {
        Rib* rib = m_context.get_rib(node.spec());
        if (!rib) {
            log::error("unresolved rib specifier: '" + node.spec() + "'",
                log::Span { m_rib->path(), node.span() });

            return;
        }

        m_scope = rib->scope();
    }

    Symbol symbol = {};
    if (!m_scope->get(node.name(), symbol)) {
        log::error("unresolved reference: '" + node.name() + "'",
            log::Span { m_rib->path(), node.span() });

        m_scope = prev_scope;
        return;
    }

    m_scope = prev_scope;

    if (symbol.kind != Symbol::Kind::Definition) {
        log::error("invalid type reference: '" + node.name() + "'",
            log::Span { m_rib->path(), node.span() });

        return;
    }

    ValueDefn* vd = dynamic_cast<ValueDefn*>(symbol.defn);
    if (!vd) {
        log::error("invalid reference to non-value: '" + node.name() + "'",
            log::Span { m_rib->path(), node.span() });
        
        return;
    }

    node.set_defn(vd);
    node.set_type(vd->type());

    VisitorBase::visit(node);
}

void NameResolution::visit(SizeofExpr& node) {
    Type* type = resolve_type(node.target());
    if (!type) {
        log::error("unresolved sizeof type: '" + node.target()->string() + "'", 
            log::Span { m_rib->path(), node.span() });

        return;
    }

    node.set_target(type);

    VisitorBase::visit(node);
}

void NameResolution::visit(StructInitExpr& node) {
    const log::Span span = { m_rib->path(), node.span() };

    Type* type = resolve_type(node.type());
    if (!type) {
        log::error("unresolved struct type: '" + node.type()->string() + "'", span);
        return;
    }

    StructType* st = dynamic_cast<StructType*>(type);
    if (!st) {
        log::error("invalid struct type: '" + type->string() + "'", span);
        return;
    }

    node.set_type(st);

    StructDefn* sd = st->defn();
    assert(sd);

    VisitorBase::visit(node);

    for (FieldInitExpr* fi : node.fields()) {
        FieldDefn* field = sd->get_field(fi->name());
        if (!field) {
            log::error("field '" + fi->name() + "' does not exist in '" + st->string() + "'", span);
            return;
        }

        fi->set_field(field);
    }
}

Type* NameResolution::resolve_type(Type* type) const {
    if (auto deferred = dynamic_cast<DeferredType*>(type)) {
        Symbol symbol = {};
        if (!m_scope->get(deferred->name(), symbol))
            return nullptr;

        if (symbol.kind == Symbol::Kind::Type) {
            return symbol.type;
        } else if (symbol.kind == Symbol::Kind::Definition) {
            TypeDefn* td = dynamic_cast<TypeDefn*>(symbol.defn);
            if (!td)
                return nullptr;

            return td->type();
        }

        return nullptr;
    } else if (auto ft = dynamic_cast<FunctionType*>(type)) {
        Type* result = resolve_type(ft->result());
        if (!result)
            return nullptr;

        std::vector<Type*> params = {};
        params.reserve(ft->num_params());

        for (Type* param : ft->params()) {
            Type* res = resolve_type(param);
            if (!res)
                return nullptr;

            params.push_back(res);
        }

        return FunctionType::get(*m_rib, result, params);
    } else if (auto pt = dynamic_cast<PointerType*>(type)) {
        Type* pointee = resolve_type(pt->pointee());
        if (!pointee)
            return nullptr;

        // Only update the pointee if the resolved type is different.
        if (pointee != pt->pointee())
            pt->set_pointee(pointee);

        return pt;
    }

    return type;
}
