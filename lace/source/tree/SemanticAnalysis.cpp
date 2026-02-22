//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/SemanticAnalysis.h"
#include "lace/tree/Stmt.h"
#include "lace/tree/Type.h"
#include "lace/tree/VisitorBase.h"

#include <cassert>

using namespace lace;

/// Test if the given |type| can be evaluated to a boolean (either trivially or 
/// through a comparison).
static inline bool is_boolean_evaluable(const Type* type) {
    return type->is_integer() 
        || type->is_floating_point() 
        || dynamic_cast<const PointerType*>(type);
}

SemanticAnalysis::TypeCheckResult SemanticAnalysis::type_check(
        const Type* actual, const Type* expected, TypeCheckMode mode) const {
    if (actual->compare(expected))
        return Match;

    switch (mode) 
    {
    case Explicit:
        return Mismatch;

    case AllowImplicit:
        if (actual->can_cast(expected, true))
            return Cast;

        return Mismatch;

    case Loose:
        if (actual->can_cast(expected, true))
            return Cast;

        auto actual_ptr = dynamic_cast<const PointerType*>(actual);
        auto expected_ptr = dynamic_cast<const PointerType*>(expected);

        if (actual->is_integer() && expected_ptr) {
            return Match;
        } else if (actual_ptr && expected->is_integer()) {
            return Match;
        }

        return Mismatch;
    }
}

SemanticAnalysis::SemanticAnalysis(Options& options) : VisitorBase(options) {}

void SemanticAnalysis::visit(VariableDefn& node) {
    if (node.has_init()) {
        Expr* init = node.init();
        init->accept(*this);

        const log::Span span = { m_ast->get_file(), node.span() };
        if (node.is_global() && !init->is_constant())
            log::fatal("globals cannot be initialized with non-constants", span);

        const Type* actual = init->type();
        const Type* expected = node.type();

        TypeCheckResult res = type_check(actual, expected);
        if (res == TypeCheckResult::Mismatch) {
            log::fatal("initializer type mismatch; got " + actual->string() 
                + ", but expected " + expected->string(), span);
        } else if (res == TypeCheckResult::Cast) {
            node.m_init = CastExpr::create(
                *m_ast, 
                init->get_span(), 
                node.type(), 
                init
            );
        }
    }
}

void SemanticAnalysis::visit(FunctionDefn& node) {
    m_func = &node;

    const log::Span span = { m_ast->get_file(), node.span() };

    if (node.is_main()) {
        if (!node.has_rune(Rune::Kind::Public))
            log::error("'main' must be marked with $public", span);

        const Type* result = node.get_return_type();
        const Type* s64 = BuiltinType::get(*m_ast, BuiltinType::Kind::Int64);
        if (!result->compare(s64))
            log::error("'main' must return 's64'", span);
    }
    
    if (node.has_body())
        node.body()->accept(*this);

    m_func = nullptr;
}

void SemanticAnalysis::visit(IfStmt& node) {
    const log::Span span = { m_ast->get_file(), node.get_span() };

    VisitorBase::visit(node);

    // Check that the if condition can be evaluated to a boolean.
    if (!is_boolean_evaluable(node.condition()->type()))
        log::fatal("'if' condition must be a boolean", span);
}

void SemanticAnalysis::visit(RestartStmt& node) {
    const log::Span span = { m_ast->get_file(), node.get_span() };

    // Check that restart statements are inside loop bodies.
    if (m_loop == None)
        log::fatal("'restart' outside of loop", span);
}

void SemanticAnalysis::visit(RetStmt& node) {
    const log::Span span = { m_ast->get_file(), node.get_span() };

    VisitorBase::visit(node);

    if (!m_func)
        log::fatal("'ret' outside of function", span);
 
    if (!node.has_expr()) {
        if (!m_func->get_return_type()->is_void())
            log::fatal("function does not return 'void'", span);

        return;
    }

    Expr* expr = node.expr();

    const Type* actual = expr->type();
    const Type* expected = m_func->get_return_type();

    TypeCheckResult res = type_check(actual, expected);
    if (res == TypeCheckResult::Mismatch) {
        log::fatal("return type mismatch; got " + actual->string(), span);
    } else if (res == TypeCheckResult::Cast) {
        node.m_expr = CastExpr::create(
            *m_ast, 
            expr->get_span(), 
            m_func->get_return_type(), 
            expr
        );
    }
}

void SemanticAnalysis::visit(StopStmt& node) {
    const log::Span span = { m_ast->get_file(), node.get_span() };

    // Check that stop statements are inside loop bodies.
    if (m_loop == None)
        log::fatal("'stop' outside of loop", span);
}

void SemanticAnalysis::visit(UntilStmt& node) {
    const log::Span span = { m_ast->get_file(), node.get_span() };

    Expr* cond = node.condition();
    cond->accept(*this);

    // Check that the while condition can be evaluated to a boolean.
    if (!is_boolean_evaluable(cond->type()))
        log::fatal("'until' condition must be a boolean", span);

    if (node.has_body()) {
        Loop prev_loop = m_loop;

        m_loop = Until;
        node.body()->accept(*this);

        m_loop = prev_loop;
    }
}

void SemanticAnalysis::visit(BinaryOp& node) {
    VisitorBase::visit(node);

    const log::Span span = { m_ast->get_file(), node.get_span() };

    Expr* lhs = node.lhs();
    Expr* rhs = node.rhs();

    const Type* lhs_type = lhs->type();
    const Type* rhs_type = rhs->type();

    BinaryOp::Operator op = node.op();
    bool supports_ptr_arith = op == BinaryOp::Add || op == BinaryOp::Sub;

    TypeCheckMode mode = supports_ptr_arith ? Loose : AllowImplicit;
    TypeCheckResult res = type_check(rhs_type, lhs_type, mode);
    if (res == TypeCheckResult::Mismatch) {
        log::fatal("operand type mismatch; got " + rhs_type->string(), span);
    } else if (res == TypeCheckResult::Cast) {
        node.m_rhs = CastExpr::create(
            *m_ast, rhs->get_span(), lhs->type(), rhs);
    }

    // Set the resulting type of the operator to a 'bool' if the operator is
    // a boolean comparison.
    if (BinaryOp::is_comparison(op)) {
        node.set_type(BuiltinType::get(*m_ast, BuiltinType::Kind::Bool));
        return;
    } else {
        // Default the type of the operator to the LHS type.
        node.set_type(lhs->type());
    }

    // Check that left hand operands of assignments are lvalues.
    if (BinaryOp::is_assignment(op) && !lhs->is_lvalue())
        log::fatal("left hand operand must be an lvalue", span);
}

void SemanticAnalysis::visit(UnaryOp& node) {
    VisitorBase::visit(node);

    node.set_type(node.expr()->type());

    const log::Span span = { m_ast->get_file(), node.get_span() };
    const Type* type = node.type();

    switch (node.op()) 
    {
    case UnaryOp::Negate:
        // Check operator type compatibility (numerics only).
        if (!(type->is_integer() || type->is_floating_point()))
            log::fatal("'-' operator incompatible with " + type->string(), span);

        node.set_type(node.type());
        break;

    case UnaryOp::Not:
        // Check operator type compatibility (integers only).
        if (!type->is_integer())
            log::fatal("'~' operator incompatible with " + type->string(), span);

        node.set_type(node.type());
        break;

    case UnaryOp::LogicNot:
        // Check operator type compatibility (scalar only).
        if (!type->is_integer() && !type->is_floating_point() && !dynamic_cast<const PointerType*>(type))
            log::fatal("'!' operator incompatible with " + type->string(), span);

        node.set_type(node.type());
        break;

    case UnaryOp::AddressOf: {
        if (!node.expr()->is_lvalue())
            log::fatal("'&' base must be an lvalue", span);

        node.set_type(PointerType::get(*m_ast, node.type()));
        break;
    }

    case UnaryOp::Dereference: {
        auto ptr = dynamic_cast<PointerType*>(node.type());
        if (!ptr)
            log::fatal("'*' operator incompatible with " + type->string(), span);

        node.set_type(ptr->pointee());
        break;
    }

    case UnaryOp::Unknown:
        log::fatal("unknown unary operator", span);
    }
}

void SemanticAnalysis::visit(CastExpr& node) {
    VisitorBase::visit(node);

    const log::Span span = { m_ast->get_file(), node.get_span() };

    if (!node.expr()->type()->can_cast(node.type()))
        log::fatal("unsupported cast", span);
}

void SemanticAnalysis::visit(ParenExpr& node) {
    VisitorBase::visit(node);

    node.set_type(node.expr()->type());
}

void SemanticAnalysis::visit(AccessExpr& node) {
    VisitorBase::visit(node);

    FieldDefn* field = node.field();
    assert(field);

    node.set_type(field->type());
}

void SemanticAnalysis::visit(SubscriptExpr& node) {
    VisitorBase::visit(node);

    const log::Span span = { m_ast->get_file(), node.get_span() };

    Expr* base = node.base();
    Expr* index = node.index();

    if (auto ptr = dynamic_cast<PointerType*>(base->type())) {
        node.set_type(ptr->pointee());
    } else {
        log::fatal("invalid argument type to '[]' operator: " 
            + base->type()->string(), span);
    }
}

void SemanticAnalysis::visit(RefExpr& node) {
    ValueDefn* defn = node.defn();
    assert(defn);

    node.set_type(defn->type());
}

void SemanticAnalysis::visit(CallExpr& node) {
    Expr* callee = node.callee();
    callee->accept(*this);

    const log::Span span = { m_ast->get_file(), node.get_span() };

    FunctionType* sig = dynamic_cast<FunctionType*>(node.callee()->type());
    if (!sig)
        log::fatal("callee is not a function", span);

    node.set_type(sig->result());

    if (node.num_args() != sig->num_params())
        log::fatal("argument count mismatch, expected " + 
            std::to_string(sig->num_params()), span);

    // Pass over each argument and compare its type to the functions expected
    // parameter type.
    for (uint32_t i = 0, e = node.num_args(); i < e; ++i) {
        Expr* arg = node.get_arg(i);
        arg->accept(*this);

        const Type* actual = arg->type();
        const Type* expected = sig->get_param(i);

        TypeCheckResult res = type_check(actual, expected);
        if (res == TypeCheckResult::Mismatch) {
            log::fatal("argument type mismatch; got " + actual->string(), span);
        } else if (res == TypeCheckResult::Cast) {
            node.m_args[i] = CastExpr::create(
                *m_ast, 
                arg->get_span(), 
                sig->get_param(i), 
                arg
            );
        }
    }
}

void SemanticAnalysis::visit(StructInitExpr& node) {
    VisitorBase::visit(node);

    const log::Span span = log::Span(m_ast->get_file(), node.get_span());

    StructType* type = dynamic_cast<StructType*>(node.type());
    assert(type);

    StructDefn* defn = type->defn();

    for (auto& [field_name, expr] : node.fields()) {
        FieldDefn* field = defn->get_field(field_name);
        assert(field);

        TypeCheckResult res = type_check(expr->type(), field->type());
        if (res == TypeCheckResult::Mismatch) {
            log::fatal("argument type mismatch, got " + expr->type()->string(), span);
        } else if (res == TypeCheckResult::Cast) {
            node.fields()[field_name] = CastExpr::create(
                *m_ast, 
                expr->get_span(), 
                field->type(), 
                expr
            );
        }
    }
}
