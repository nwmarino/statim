//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Stmt.h"
#include "lace/tree/Rib.h"
#include "lace/tree/Scope.h"
#include "lace/tree/SymbolAnalysis.h"
#include "lace/tree/VisitorBase.h"

using namespace lace;

SymbolAnalysis::SymbolAnalysis(Options& options) : VisitorBase(options) {}

void SymbolAnalysis::visit(Rib& rib) {
    m_scope = rib.scope();

    VisitorBase::visit(rib);
}

void SymbolAnalysis::visit(AliasDefn& node) {
    assert(m_scope);

    Symbol::Visibility visibility = Symbol::Visibility::Private;
    if (node.has_rune(Rune::Kind::Public))
        visibility = Symbol::Visibility::Public;

    bool res = m_scope->add(Symbol {
        .name = node.name(),
        .qual_name = qualify_name(node.name()),
        .kind = Symbol::Kind::Definition,
        .visibility = visibility,
        .defn = &node,
    });

    if (!res) {
        log::error("alias conflicts with existing name: " + node.name(), 
            log::Span { m_rib->path(), node.span() });
    }
}

void SymbolAnalysis::visit(EnumDefn& node) {
    assert(m_scope);

    Symbol::Visibility visibility = Symbol::Visibility::Private;
    if (node.has_rune(Rune::Kind::Public))
        visibility = Symbol::Visibility::Public;

    bool res = m_scope->add(Symbol {
        .name = node.name(),
        .qual_name = qualify_name(node.name()),
        .kind = Symbol::Kind::Definition,
        .visibility = visibility,
        .defn = &node,
    });

    if (!res) {
        log::error("enum conflicts with existing name: " + node.name(), 
            log::Span { m_rib->path(), node.span() });
    }

    VisitorBase::visit(node);
}

void SymbolAnalysis::visit(FunctionDefn& node) {
    assert(m_scope);

    Symbol::Visibility visibility = Symbol::Visibility::Private;
    if (node.has_rune(Rune::Kind::Public))
        visibility = Symbol::Visibility::Public;

    bool res = m_scope->add(Symbol {
        .name = node.name(),
        .qual_name = qualify_name(node.name()),
        .kind = Symbol::Kind::Definition,
        .visibility = visibility,
        .defn = &node,
    });

    if (!res) {
        log::error("function conflicts with existing name: " + node.name(), 
            log::Span { m_rib->path(), node.span() });
    }

    node.scope()->set_parent(m_scope);
    m_scope = node.scope();

    VisitorBase::visit(node);

    m_scope = m_scope->parent();
}

void SymbolAnalysis::visit(ParameterDefn& node) {
    assert(m_scope);

    Symbol::Visibility visibility = Symbol::Visibility::Private;
    if (node.has_rune(Rune::Kind::Public))
        visibility = Symbol::Visibility::Public;

    bool res = m_scope->add(Symbol {
        .name = node.name(),
        .qual_name = qualify_name(node.name()),
        .kind = Symbol::Kind::Definition,
        .visibility = visibility,
        .defn = &node,
    });

    if (!res) {
        log::error("parameter conflicts with existing name: " + node.name(), 
            log::Span { m_rib->path(), node.span() });
    }
}

void SymbolAnalysis::visit(StructDefn& node) {
    assert(m_scope);

    Symbol::Visibility visibility = Symbol::Visibility::Private;
    if (node.has_rune(Rune::Kind::Public))
        visibility = Symbol::Visibility::Public;

    bool res = m_scope->add(Symbol {
        .name = node.name(),
        .qual_name = qualify_name(node.name()),
        .kind = Symbol::Kind::Definition,
        .visibility = visibility,
        .defn = &node,
    });

    if (!res) {
        log::error("struct conflicts with existing name: " + node.name(), 
            log::Span { m_rib->path(), node.span() });
    }

    VisitorBase::visit(node);
}

void SymbolAnalysis::visit(VariableDefn& node) {
    assert(m_scope);

    Symbol::Visibility visibility = Symbol::Visibility::Private;
    if (node.has_rune(Rune::Kind::Public))
        visibility = Symbol::Visibility::Public;

    bool res = m_scope->add(Symbol {
        .name = node.name(),
        .qual_name = node.is_global() ? qualify_name(node.name()) : node.name(),
        .kind = Symbol::Kind::Definition,
        .visibility = visibility,
        .defn = &node,
    });

    if (!res) {
        log::error("variable conflicts with existing name: " + node.name(), 
            log::Span { m_rib->path(), node.span() });
    }

    VisitorBase::visit(node);
}

void SymbolAnalysis::visit(VariantDefn& node) {
    assert(m_scope);

    Symbol::Visibility visibility = Symbol::Visibility::Private;
    if (node.has_rune(Rune::Kind::Public))
        visibility = Symbol::Visibility::Public;

    bool res = m_scope->add(Symbol {
        .name = node.name(),
        .qual_name = qualify_name(node.name()),
        .kind = Symbol::Kind::Definition,
        .visibility = visibility,
        .defn = &node,
    });

    if (!res) {
        log::error("enum variant conflicts with existing name: " + node.name(), 
            log::Span { m_rib->path(), node.span() });
    }
}

void SymbolAnalysis::visit(BlockStmt& node) {
    assert(m_scope);

    node.scope()->set_parent(m_scope);
    m_scope = node.scope();

    VisitorBase::visit(node);

    m_scope = m_scope->parent();
}

std::string SymbolAnalysis::qualify_name(const std::string& name) const {
    std::string res = name;
    
    Rib* curr = m_rib;
    while (curr)
        res = curr->name() + '.' + res;
    
    return res;
}
