//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_SYMBOL_ANALYSIS_H_
#define LOVELACE_SYMBOL_ANALYSIS_H_

//
//  This header file declares a syntax tree analysis pass to perform certain
//  symbol-related checks like name and deferred type resolution, as well as
//  type propogation in some cases.
//

#include "lace/core/Options.hpp"
#include "lace/tree/Scope.hpp"
#include "lace/tree/Type.hpp"
#include "lace/tree/VisitorBase.hpp"

namespace lace {

class SymbolAnalysis final : public VisitorBase {
    const Options& m_options;
    
    AST* m_ast = nullptr;
    AST::Context* m_context = nullptr;
    const Scope* m_scope = nullptr;

    /// Replace all deferred types composed in |type| and return the new,
    /// fully resolved type. If a part could not be resolved, then null is
    /// returned.
    bool resolve_type(const QualType& type) const;

public:
    SymbolAnalysis(const Options& options);

    void visit(AST& ast) override;

    void visit(VariableDefn& node) override;
    void visit(FunctionDefn& node) override;
    void visit(StructDefn& node) override;

    void visit(AdapterStmt& node) override;
    void visit(BlockStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(UntilStmt& node) override;
    void visit(RuneStmt& node) override;
    
    void visit(BinaryOp& node) override;
    void visit(UnaryOp& node) override;

    void visit(AccessExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(CastExpr& node) override;
    void visit(ParenExpr& node) override;
    void visit(RefExpr& node) override;
    void visit(SizeofExpr& node) override;
    void visit(SubscriptExpr& node) override;
};

} // namespace lace

#endif // LOVELACE_SYMBOL_ANALYSIS_H_
