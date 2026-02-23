//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_SYMBOL_ANALYSIS_H_
#define LACE_SYMBOL_ANALYSIS_H_

//
//  This header file declares a syntax tree analysis pass to perform certain
//  symbol-related checks like name and deferred type resolution, as well as
//  type propogation in some cases.
//

#include "lace/tree/VisitorBase.h"

namespace lace {

class SymbolAnalysis final : public VisitorBase {
public:
    SymbolAnalysis(Options& options);

    void visit(VariableDefn& node) override;

    void visit(AccessExpr& node) override;
    
    void visit(CallExpr& node) override;

    void visit(CastExpr& node) override;

    void visit(RefExpr& node) override;

    void visit(SizeofExpr& node) override;

    void visit(SpecifierExpr& node) override;

    void visit(StructInitExpr& node) override;
};

} // namespace lace

#endif // LACE_SYMBOL_ANALYSIS_H_
