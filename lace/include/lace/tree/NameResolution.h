//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_NAME_RESOLUTION_H_
#define LACE_NAME_RESOLUTION_H_

//
//  This header file declares the NameResolution syntax tree pass, whose 
//  purpose is to resolve names specified by source through the scope trees
//  created by SymbolAnalysis.
//
//  Any node which may potentially reference a name (be it as a value, type,
//  or target otherwise) should override special behavior within this pass.
//

#include "lace/tree/VisitorBase.h"

namespace lace {

class NameResolution final : public VisitorBase {
public:
    NameResolution(Options& options);

    void visit(Rib& rib) override;

    void visit(AliasDefn& node) override;

    void visit(EnumDefn& node) override;

    void visit(FunctionDefn& node) override;

    void visit(ParameterDefn& node) override;

    void visit(VariableDefn& node) override;

    void visit(AccessExpr& node) override;

    void visit(CastExpr& node) override;

    void visit(RefExpr& node) override;

    void visit(SizeofExpr& node) override;

    void visit(StructInitExpr& node) override;
};

} // namespace lace

#endif // LACE_NAME_RESOLUTION_H_
