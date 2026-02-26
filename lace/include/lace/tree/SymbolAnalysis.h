//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_SYMBOL_ANALYSIS_H_
#define LACE_SYMBOL_ANALYSIS_H_

//
//  This header file declares a syntax tree analysis pass which fully qualifies
//  names and constructs scope trees for an entire source file.
//

#include "lace/tree/VisitorBase.h"

namespace lace {

class SymbolAnalysis final : public VisitorBase {
public:
    SymbolAnalysis(Options& options);

    void visit(Rib& rib) override;

    void visit(AliasDefn& node) override;

    void visit(EnumDefn& node) override;

    void visit(FunctionDefn& node) override;

    void visit(ParameterDefn& node) override;

    void visit(StructDefn& node) override;

    void visit(VariableDefn& node) override;

    void visit(VariantDefn& node) override;

    void visit(BlockStmt& node) override;
};

} // namespace lace

#endif // LACE_SYMBOL_ANALYSIS_H_
