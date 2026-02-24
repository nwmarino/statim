//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_SEMANTIC_ANALYSIS_H_
#define LACE_SEMANTIC_ANALYSIS_H_

//
//  This header file declares a syntax tree analysis pass that performs
//  numerous semantic checks, e.g. type checking, implicit casting, control
//  flow constructs, and more.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/VisitorBase.h"

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
    
    Loop m_loop = None;
    FunctionDefn* m_func = nullptr;

    TypeCheckResult type_check(const Type* actual, const Type* expected, 
                               TypeCheckMode mode = AllowImplicit) const;

public:
    SemanticAnalysis(Options& options);

    void visit(VariableDefn& node) override;
    void visit(FunctionDefn& node) override;
    
    void visit(IfStmt& node) override;
    void visit(RestartStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(StopStmt& node) override;
    void visit(UntilStmt& node) override;

    void visit(BinaryOp& node) override;
    void visit(UnaryOp& node) override;

    void visit(AccessExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(CastExpr& node) override;
    void visit(ParenExpr& node) override;
    void visit(RefExpr& node) override;
    void visit(StructInitExpr& node) override;
    void visit(SubscriptExpr& node) override;
};

} // namespace lace

#endif // LACE_SEMANTIC_ANALYSIS_H_
