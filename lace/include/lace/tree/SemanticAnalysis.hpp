//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_SEMANTIC_ANALYSIS_H_
#define LOVELACE_SEMANTIC_ANALYSIS_H_

//
//  This header file declares a syntax tree analysis pass that performs
//  numerous semantic checks, e.g. type checking, implicit casting, control
//  flow constructs, and more.
//

#include "lace/core/Options.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Expr.hpp"
#include "lace/tree/VisitorBase.hpp"

namespace lace {

class SemanticAnalysis final : public VisitorBase {
    /// The different kinds of loops.
    enum Loop : uint32_t {
        None = 0,
        Until,
    };

    /// The different modes for a type check.
    enum TypeCheckMode : uint32_t {
        Explicit,
        Loose,
        AllowImplicit,
    };

    /// The different results of a type check.
    enum TypeCheckResult : uint32_t {
        Match,
        Mismatch,
        Cast,
    };
    
    const Options& m_options;
    
    Loop m_loop = None;

    AST* m_ast = nullptr;
    AST::Context* m_context = nullptr;
    FunctionDefn* m_function = nullptr;
    const Scope* m_scope = nullptr;

    TypeCheckResult type_check(const QualType& actual, const QualType& expected, 
                               TypeCheckMode mode = AllowImplicit) const;

public:
    SemanticAnalysis(const Options& options);

    void visit(AST& ast) override;

    void visit(VariableDefn& node) override;
    void visit(FunctionDefn& node) override;
    
    void visit(AdapterStmt& node) override;
    void visit(BlockStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(RestartStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(StopStmt& node) override;
    void visit(UntilStmt& node) override;
    void visit(RuneStmt& node) override;

    void visit(BinaryOp& node) override;
    void visit(UnaryOp& node) override;

    void visit(AccessExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(CastExpr& node) override;
    void visit(ParenExpr& node) override;
    void visit(RefExpr& node) override;
    void visit(SubscriptExpr& node) override;
};

} // namespace lace

#endif // LOVELACE_SEMANTIC_ANALYSIS_H_
