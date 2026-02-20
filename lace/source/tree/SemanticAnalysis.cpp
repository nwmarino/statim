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

/// Test if |type| can be evaluated to a boolean (either trivially or through 
/// a comparison).
static inline bool is_boolean_evaluable(const QualType& type) {
    const Type* ty = type.getType();
    return ty->isInteger() || ty->isFloatingPoint() || ty->isClass(Type::Class::Pointer);
}

SemanticAnalysis::TypeCheckResult SemanticAnalysis::type_check(
        const QualType& actual, const QualType& expected, 
        SemanticAnalysis::TypeCheckMode mode) const {
    if (actual.compare(expected))
        return Match;

    switch (mode) {
        case Explicit:
            return Mismatch;

        case AllowImplicit:
            if (actual.canCast(expected, true))
                return Cast;

            return Mismatch;

        case Loose:
            if (actual.canCast(expected, true))
                return Cast;

            if ((actual->isInteger() && expected->isClass(Type::Class::Pointer)) 
            || (actual->isClass(Type::Class::Pointer) && expected->isInteger()))
                return Match;

            return Mismatch;
    }
}

SemanticAnalysis::SemanticAnalysis(Options& options) : VisitorBase(options) {}

void SemanticAnalysis::visit(VariableDefn& node) {
    if (node.has_init()) {
        Expr* init = node.get_init();
        init->accept(*this);

        const log::Span span = log::Span(m_ast->get_file(), node.get_span());
        if (node.is_global() && !init->is_constant())
            log::fatal("globals cannot be initialized with non-constants", span);

        init->get_type().withMut();

        const QualType& actual = init->get_type();
        const QualType& expected = node.get_type();

        TypeCheckResult TC = type_check(actual, expected);
        if (TC == TypeCheckResult::Mismatch) {
            log::fatal("initializer type mismatch; got " + actual->string() 
                + ", but expected " + expected->string(), span);
        } else if (TC == TypeCheckResult::Cast) {
            node.m_init = CastExpr::create(
                m_ast->get_context(), 
                init->get_span(), 
                expected, 
                init
            );
        }
    }
}

void SemanticAnalysis::visit(FunctionDefn& node) {
    m_function = &node;

    if (node.is_main()) {
        if (!node.hasRune(Rune::Public)) {
            log::error("'main' must be marked with $public", 
                log::Span(m_ast->get_file(), node.get_span().start));
        }

        const QualType& ret_type = node.get_return_type();
        if (!ret_type.compare(BuiltinType::get(m_ast->get_context(), BuiltinType::Kind::Int64))) {
            log::error("'main' must return 's64'", 
                log::Span(m_ast->get_file(), node.get_span().start));
        }
    }
    
    if (node.has_body())
        node.get_body()->accept(*this);

    m_function = nullptr;
}

void SemanticAnalysis::visit(IfStmt& node) {
    VisitorBase::visit(node);

    Expr* cond = node.get_cond();

    // Check that the if condition can be evaluated to a boolean.
    if (!is_boolean_evaluable(cond->get_type().getType()))
        log::fatal("'if' condition must be a boolean", 
            log::Span(m_ast->get_file(), cond->get_span()));
}

void SemanticAnalysis::visit(RestartStmt& node) {
    // Check that restart statements are inside loop bodies.
    if (m_loop == None)
        log::fatal("'restart' outside of loop", log::Span(m_ast->get_file(), node.get_span()));
}

void SemanticAnalysis::visit(RetStmt& node) {
    VisitorBase::visit(node);

    const log::Span span = log::Span(m_ast->get_file(), node.get_span());
    if (!m_function)
        log::fatal("'ret' outside of function", span);
 
    if (!node.has_expr()) {
        if (!m_function->get_return_type()->isVoid())
            log::fatal("function does not return 'void'", span);

        return;
    }

    Expr* expr = node.get_expr();

    const QualType& val_type = expr->get_type();
    const QualType& ret_type = m_function->get_return_type();
    TypeCheckResult TC = type_check(val_type, ret_type);
    if (TC == TypeCheckResult::Mismatch) {
        log::fatal("return type mismatch; got " + val_type.string(), span);
    } else if (TC == TypeCheckResult::Cast) {
        node.m_expr = CastExpr::create(
            m_ast->get_context(), 
            expr->get_span(), 
            ret_type, 
            expr
        );
    }
}

void SemanticAnalysis::visit(StopStmt& node) {
    // Check that stop statements are inside loop bodies.
    if (m_loop == None)
        log::fatal("'stop' outside of loop", log::Span(m_ast->get_file(), node.get_span()));
}

void SemanticAnalysis::visit(UntilStmt& node) {
    Expr* cond = node.get_cond();
    cond->accept(*this);

    // Check that the while condition can be evaluated to a boolean.
    if (!is_boolean_evaluable(cond->get_type()))
        log::fatal("'until' condition must be a boolean",
            log::Span(m_ast->get_file(), cond->get_span()));

    if (node.has_body()) {
        Loop prev_loop = m_loop;
        m_loop = Until;
        node.get_body()->accept(*this);

        m_loop = prev_loop;
    }
}

void SemanticAnalysis::visit(BinaryOp& node) {
    VisitorBase::visit(node);

    Expr* lhs = node.get_lhs();
    Expr* rhs = node.get_rhs();

    const log::Span span = log::Span(m_ast->get_file(), node.get_span());
    const QualType& lhs_type = lhs->get_type();
    const QualType& rhs_type = rhs->get_type();

    BinaryOp::Operator op = node.get_operator();
    bool supports_ptr_arith = op == BinaryOp::Add || op == BinaryOp::Sub;

    TypeCheckMode mode = supports_ptr_arith ? Loose : AllowImplicit;
    TypeCheckResult TC = type_check(rhs_type, lhs_type, mode);
    if (TC == TypeCheckResult::Mismatch) {
        log::fatal("operand type mismatch; got " + rhs_type->string(), span);
    } else if (TC == TypeCheckResult::Cast) {
        node.m_rhs = CastExpr::create(
            m_ast->get_context(), rhs->get_span(), lhs_type, rhs);
    }

    // Set the resulting type of the operator to a 'bool' if the operator is
    // a boolean comparison.
    if (BinaryOp::is_comparison(op)) {
        node.set_type(BuiltinType::get(m_ast->get_context(), BuiltinType::Kind::Bool));
        return;
    } else {
        // Default the type of the operator to the LHS type.
        node.set_type(lhs_type);
    }

    if (BinaryOp::is_assignment(op)) {
        // Check that left hand operands of assignments are lvalues.
        if (!lhs->is_lvalue())
            log::fatal("left hand operand must be an lvalue", span);
    
        // Check that left hand operands of assignments are mutable.
        if (!lhs_type.isMut())
            log::fatal("left hand operand must be mutable", span);
    }
}

void SemanticAnalysis::visit(UnaryOp& node) {
    Expr* expr = node.get_expr();
    expr->accept(*this);

    const log::Span span = log::Span(m_ast->get_file(), node.get_span());
    const QualType& type = expr->get_type();

    switch (node.get_operator()) {
        /*
        case UnaryOp::Increment: {
            // Check operator type compatibility (numerics and pointers only).
            if (!(type->isInteger() || type->isFloatingPoint() || type->is_pointer()))
                m_diags.fatal("'++' operator incompatible with '" + 
                    type->string() + "'", span);
            
            if (!expr->is_lvalue())
                m_diags.fatal("'++' base must be an lvalue", span);

            if (!type.isMut())
                m_diags.fatal("'++' base must be mutable", span);

            node.set_type(type);
            break;
        }

        case UnaryOp::Decrement: {
            // Check operator type compatibility (numerics and pointers only).
            if (!(type->isInteger() || type->isFloatingPoint() || type->is_pointer()))
                m_diags.fatal("'--' operator incompatible with '" + 
                    type->string() + "'", span);
            
            if (!expr->is_lvalue())
                m_diags.fatal("'--' base must be an lvalue", span);

            if (!type.isMut())
                m_diags.fatal("'++' base must be mutable", span);

            node.set_type(type);
            break;
        }
        */
        case UnaryOp::Negate: {
            // Check operator type compatibility (numerics only).
            if (!(type->isInteger() || type->isFloatingPoint()))
                log::fatal("'-' operator incompatible with " + type->string(), span);

            node.set_type(type);
            break;
        }

        case UnaryOp::Not: {
            // Check operator type compatibility (integers only).
            if (!type->isInteger())
                log::fatal("'~' operator incompatible with " + type->string(), span);

            node.set_type(type);
            break;
        }

        case UnaryOp::LogicNot: {
            // @Todo: force scalar types arguments.
            node.set_type(type);
            break;
        }

        case UnaryOp::AddressOf: {
            if (!expr->is_lvalue())
                log::fatal("'&' base must be an lvalue", span);

            node.set_type(PointerType::get(m_ast->get_context(), type));
            break;
        }

        case UnaryOp::Dereference: {
            if (!type->isClass(Type::Class::Pointer))
                log::fatal("'*' operator incompatible with " + type->string(), span);

            node.set_type(static_cast<const PointerType*>(
                type.getType())->pointee());
            break;
        }

        case UnaryOp::Unknown:
            log::fatal("unknown unary operator", span);
    }
}

void SemanticAnalysis::visit(CastExpr& node) {
    VisitorBase::visit(node);

    if (!node.get_expr()->get_type().canCast(node.get_type()))
        log::fatal("unsupported cast", log::Span(m_ast->get_file(), node.get_span()));
}

void SemanticAnalysis::visit(ParenExpr& node) {
    VisitorBase::visit(node);

    node.set_type(node.get_expr()->get_type());
}

void SemanticAnalysis::visit(AccessExpr& node) {
    VisitorBase::visit(node);

    const FieldDefn* field = node.get_field();
    assert(field && "field access left unresolved!");

    node.set_type(field->get_type());
}

void SemanticAnalysis::visit(SubscriptExpr& node) {
    VisitorBase::visit(node);

    Expr* base = node.get_base();
    Expr* index = node.get_index();

    const Type* base_type = base->get_type().getType();
    if (auto AT = dynamic_cast<const ArrayType*>(base_type)) {
        node.set_type(AT->element());
    } else if (auto PT = dynamic_cast<const PointerType*>(base_type)) {
        node.set_type(PT->pointee());
    } else {
        const QualType& qual_type = base->get_type();

        log::fatal("invalid argument type to '[]' operator: " 
            + qual_type->string(), log::Span(m_ast->get_file(), node.get_span()));
    }
}

void SemanticAnalysis::visit(RefExpr& node) {
    const ValueDefn* defn = node.get_defn();
    assert(defn && "named reference left unresolved!");

    node.set_type(defn->get_type());
}

void SemanticAnalysis::visit(CallExpr& node) {
    Expr* callee = node.get_callee();
    callee->accept(*this);

    const log::Span span = log::Span(m_ast->get_file(), node.get_span());
    const QualType& callee_type = callee->get_type();
    const FunctionType* FT = dynamic_cast<const FunctionType*>(callee_type.getType());

    if (!FT)
        log::fatal("function call target is not a function", span);

    node.set_type(FT->result());

    if (node.num_args() != FT->numParams())
        log::fatal("argument count mismatch, expected " + 
            std::to_string(FT->numParams()), span);

    // Pass over each argument and compare its type to the functions expected
    // parameter type.
    for (uint32_t i = 0, e = node.num_args(); i < e; ++i) {
        Expr* arg = node.get_arg(i);
        arg->accept(*this);

        const QualType& actual = arg->get_type();
        const QualType& expected = FT->getParam(i);

        TypeCheckResult TC = type_check(actual, expected);
        if (TC == TypeCheckResult::Mismatch) {
            log::fatal("argument type mismatch; got " + actual->string(), span);
        } else if (TC == TypeCheckResult::Cast) {
            node.m_args[i] = CastExpr::create(
                m_ast->get_context(), arg->get_span(), expected, arg);
        }
    }
}
