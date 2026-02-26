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
    Scope* m_scope = nullptr;

public:
    SymbolAnalysis(Options& options);

    ~SymbolAnalysis() = default;

    SymbolAnalysis(const SymbolAnalysis&) = delete;
    void operator=(const SymbolAnalysis&) = delete;

    SymbolAnalysis(SymbolAnalysis&&) noexcept = delete;
    void operator=(SymbolAnalysis&&) noexcept = delete;

    void visit(Rib& rib) override;

    void visit(AliasDefn& node) override;

    void visit(EnumDefn& node) override;

    void visit(FunctionDefn& node) override;

    void visit(ParameterDefn& node) override;

    void visit(StructDefn& node) override;

    void visit(VariableDefn& node) override;

    void visit(VariantDefn& node) override;

    void visit(BlockStmt& node) override;

private:
    /// Returns the qualified version of the given |name| under the current rib
    /// tree.
    std::string qualify_name(const std::string& name) const;
};

} // namespace lace

#endif // LACE_SYMBOL_ANALYSIS_H_
