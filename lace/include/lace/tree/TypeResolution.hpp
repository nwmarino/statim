//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_TYPE_RESOLUTION_H_
#define LACE_TYPE_RESOLUTION_H_

//
//  This header file declares the TypeResolution class, which implements an invasive syntax tree 
//  pass whose goal is to fully resolve the types of any top-level definition in a compilation 
//  unit.
//
//  The nuance here is that it only performs the resolution on top-level definitions, and does not
//  traverse deeper into the tree. This is so that during dependency resolution, imported symbols
//  will have full type information.
// 
//  The pass works by visiting typed definitions at the top-level, and recursing into composed 
//  types e.g. pointers, structures, etc., and replacing any instances of the DeferredType with
//  a concrete type available in the current scope.
//

#include "lace/core/Options.hpp"
#include "lace/tree/VisitorBase.hpp"

namespace lace {

class TypeResolution final : public VisitorBase {
public:
    TypeResolution(Options& options);

    void visit(VariableDefn& node) override;

    void visit(FunctionDefn& node) override;

    void visit(FieldDefn& node) override;

    void visit(VariantDefn& node) override;
};

} // namespace lace

#endif // LACE_TYPE_RESOLUTION_H_
