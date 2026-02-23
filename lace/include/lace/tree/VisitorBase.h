//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_VISITOR_BASE_H_
#define LACE_VISITOR_BASE_H_

#include "lace/core/Options.h"
#include "lace/tree/Scope.h"
#include <vector>

namespace lace {

class AST;

class AliasDefn;
class EnumDefn;
class FieldDefn;
class FunctionDefn;
class LoadDefn;
class ParameterDefn;
class SpaceDefn;
class StructDefn;
class VariableDefn;
class VariantDefn;

class AdapterStmt;
class BlockStmt;
class IfStmt;
class RestartStmt;
class RetStmt;
class StopStmt;
class UntilStmt;
class RuneStmt;

class BoolLiteral;
class CharLiteral;
class IntegerLiteral;
class FloatLiteral;
class NullLiteral;
class StringLiteral;

class BinaryOp;
class UnaryOp;

class AccessExpr;
class CallExpr;
class CastExpr;
class ParenExpr;
class RefExpr;
class SizeofExpr;
class StructInitExpr;
class SubscriptExpr;

class Type;

class VisitorBase {
protected:
    Options& m_options;
    AST* m_ast = nullptr;
    Scope* m_scope = nullptr;
    std::vector<SpaceDefn*> m_namespaces = {};

    VisitorBase(Options& options) : m_options(options) {}

public:
    virtual ~VisitorBase() = default;

    VisitorBase(const VisitorBase&) = delete;
    void operator=(const VisitorBase&) = delete;

    VisitorBase(VisitorBase&&) noexcept = delete;
    void operator=(VisitorBase&&) noexcept = delete;
    
    virtual void visit(AST& node);

    virtual void visit(AliasDefn& node);
    virtual void visit(EnumDefn& node);
    virtual void visit(FieldDefn& node);
    virtual void visit(FunctionDefn& node);
    virtual void visit(LoadDefn& node);
    virtual void visit(ParameterDefn& node);
    virtual void visit(SpaceDefn& node);
    virtual void visit(StructDefn& node);
    virtual void visit(VariableDefn& node);
    virtual void visit(VariantDefn& node);

    virtual void visit(AdapterStmt& node);
    virtual void visit(BlockStmt& node);
    virtual void visit(IfStmt& node);
    virtual void visit(RestartStmt& node);
    virtual void visit(RetStmt& node);
    virtual void visit(StopStmt& node);
    virtual void visit(UntilStmt& node);
    virtual void visit(RuneStmt& node);

    virtual void visit(BoolLiteral& node);
    virtual void visit(CharLiteral& node);
    virtual void visit(IntegerLiteral& node);
    virtual void visit(FloatLiteral& node);
    virtual void visit(NullLiteral& node);
    virtual void visit(StringLiteral& node);
    
    virtual void visit(BinaryOp& node);
    virtual void visit(UnaryOp& node);

    virtual void visit(AccessExpr& node);
    virtual void visit(CallExpr& node);
    virtual void visit(CastExpr& node);
    virtual void visit(ParenExpr& node);
    virtual void visit(RefExpr& node);
    virtual void visit(SizeofExpr& node);
    virtual void visit(StructInitExpr& node);
    virtual void visit(SubscriptExpr& node);

protected:
    /// Attempt to resolve any deferred types within the component(s) of the 
    /// given |type|, and return a new, fully resolved type.
    ///
    /// If a component of the given |type| could not be resolved, then null is 
    /// returned.
    [[nodiscard]] Type* resolve_type(Type* type) const;
};

} // namespace lace

#endif // LACE_VISITOR_BASE_H_
