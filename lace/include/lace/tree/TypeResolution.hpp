//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_TYPE_RESOLUTION_H_
#define LACE_TYPE_RESOLUTION_H_

#include "lace/core/Common.h"
#include "lace/core/Options.hpp"
#include "lace/tree/Scope.hpp"
#include "lace/tree/Type.hpp"
#include "lace/tree/VisitorBase.hpp"

namespace lace {

class TypeResolution final : public VisitorBase {
    Options& m_options;
    
    AST* m_ast = nullptr;
    AST::Context* m_context = nullptr;
    const Scope* m_scope = nullptr;

    /// Replace all deferred types composed in |type| with fully resolved types, and returns true.
    /// If a part of the type could not be resolved, then false is returned.
    Result resolveType(const QualType& type) const;

public:
    TypeResolution(Options& options);

    void visit(AST& ast) override;

    void visit(VariableDefn& node) override;

    void visit(FunctionDefn& node) override;

    void visit(FieldDefn& node) override;

    void visit(VariantDefn& node) override;

    void visit(StructDefn& node) override;
    
    void visit(EnumDefn& node) override;
};

} // namespace lace

#endif // LACE_TYPE_RESOLUTION_H_
