//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LACE_BASE_VISITOR_H_
#define LACE_BASE_VISITOR_H_

#include "lace/tree/Visitor.hpp"
namespace lace {

class AST;

class AliasDefn;
class EnumDefn;
class FieldDefn;
class FunctionDefn;
class LoadDefn;
class ParameterDefn;
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
class SubscriptExpr;

#define VISIT_DEFAULT { static_cast<Derived*>(this)->visitDefault(node); }

template <typename Derived> class VisitorBase {
public:
    virtual ~VisitorBase() = default;

    VisitorBase(const VisitorBase&) = delete;
    void operator=(const VisitorBase&) = delete;

    VisitorBase(VisitorBase&&) noexcept = delete;
    void operator=(VisitorBase&&) noexcept = delete;

    Derived& getDerived() { return *static_cast<Derived*>(this); }

    template<typename T> 
    void visitDefault(T& node) {
        return;
    }

    void visit(AST& node) VISIT_DEFAULT;

    void visit(AliasDefn& node) VISIT_DEFAULT;

    void visit(EnumDefn& node) VISIT_DEFAULT;

    void visit(FieldDefn& node) VISIT_DEFAULT;

    void visit(FunctionDefn& node) VISIT_DEFAULT;

    void visit(LoadDefn& node) VISIT_DEFAULT;

    void visit(ParameterDefn& node) VISIT_DEFAULT;

    void visit(StructDefn& node) VISIT_DEFAULT;

    void visit(VariableDefn& node) VISIT_DEFAULT;

    void visit(VariantDefn& node) VISIT_DEFAULT;

    void visit(AdapterStmt& node) VISIT_DEFAULT;

    void visit(BlockStmt& node) VISIT_DEFAULT;
    
    void visit(IfStmt& node) VISIT_DEFAULT;

    void visit(RestartStmt& node) VISIT_DEFAULT;

    void visit(RetStmt& node) VISIT_DEFAULT;

    void visit(StopStmt& node) VISIT_DEFAULT;

    void visit(UntilStmt& node) VISIT_DEFAULT;

    void visit(RuneStmt& node) VISIT_DEFAULT;

    void visit(BoolLiteral& node) VISIT_DEFAULT;

    void visit(CharLiteral& node) VISIT_DEFAULT;

    void visit(IntegerLiteral& node) VISIT_DEFAULT;

    void visit(FloatLiteral& node) VISIT_DEFAULT;

    void visit(NullLiteral& node) VISIT_DEFAULT;

    void visit(StringLiteral& node) VISIT_DEFAULT;

    void visit(BinaryOp& node) VISIT_DEFAULT;

    void visit(UnaryOp& node) VISIT_DEFAULT;

    void visit(AccessExpr& node) VISIT_DEFAULT;

    void visit(CallExpr& node) VISIT_DEFAULT;

    void visit(CastExpr& node) VISIT_DEFAULT;

    void visit(ParenExpr& node) VISIT_DEFAULT;

    void visit(RefExpr& node) VISIT_DEFAULT;

    void visit(SizeofExpr& node) VISIT_DEFAULT;

    void visit(SubscriptExpr& node) VISIT_DEFAULT;
};

class NameAnalysis final : public VisitorBase<NameAnalysis> {
public:
    void visit(AST& ast);
};

} // namespace lace

#endif // LACE_BASE_VISITOR_H_
