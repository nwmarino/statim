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

SemanticAnalysis::SemanticAnalysis(Context& context) : VisitorBase(context) {}

void SemanticAnalysis::visit(VariableDefn& node) {
    const log::Span span = { m_rib->path(), node.span() };
    
    VisitorBase::visit(node);

    if (!node.has_init())
        return;

    Expr* init = node.init();

    if (node.is_global() && !init->is_constant()) {
        // Unrecoverable error, but not detrimental to pass continuation.
        log::error("globals cannot be initialized with non-constants", span);
    }

    Type* actual = init->type();
    Type* expected = node.type();

    const TypeCheckResult res = type_check(actual, expected);
    if (res == Cast) {
        node.m_init = CastExpr::create(
            *m_rib,
            init->span(), 
            node.type(), 
            init
        );
    } else if (res == Mismatch) {
        log::error("initializer type mismatch; got '" + actual->string() + "', but expected '" + expected->string() + "'", span);
    }
}

void SemanticAnalysis::visit(FunctionDefn& node) {
    m_func = &node;

    const log::Span span = { m_rib->path(), node.span() };

    if (node.is_main()) {
        if (!node.has_rune(Rune::Kind::Public))
            log::error("'main' function must be marked with '$public'", span);

        const Type* result = node.get_return_type();
        const Type* s64 = BuiltinType::get(*m_rib, BuiltinType::Kind::Int64);
        
        if (!result->compare(s64))
            log::error("'main' function must return a 's64'", span);
    }
    
    VisitorBase::visit(node);

    m_func = nullptr;
}

void SemanticAnalysis::visit(IfStmt& node) {
    const log::Span span = { m_rib->path(), node.span() };

    VisitorBase::visit(node);

    // Check that the if condition can be evaluated to a boolean.
    if (!is_boolean_evaluable(node.condition()->type()))
        log::error("'if' condition must be a boolean", span);
}

void SemanticAnalysis::visit(RestartStmt& node) {
    const log::Span span = { m_rib->path(), node.span() };

    // Check that restart statements are inside loop bodies.
    if (m_loop == None)
        log::error("'restart' statement outside of loop", span);
}

void SemanticAnalysis::visit(RetStmt& node) {
    const log::Span span = { m_rib->path(), node.span() };

    VisitorBase::visit(node);

    if (!m_func) {
        log::error("'ret' statement outside of function", span);
        return;
    }

    if (!node.has_expr()) {
        if (!m_func->get_return_type()->is_void())
            log::error("'ret' has no value, function does not return 'void'", span);

        return;
    }

    Expr* expr = node.expr();
    Type* actual = expr->type();
    Type* expected = m_func->get_return_type();

    const TypeCheckResult res = type_check(actual, expected);
    if (res == TypeCheckResult::Cast) {
        node.m_expr = CastExpr::create(
            *m_rib, 
            expr->span(), 
            expected, 
            expr
        );
    } else if (res == TypeCheckResult::Mismatch) {
        log::error("return type mismatch; got '" + actual->string() + "', but expected '" + expected->string() + "'", span);
    }
}

void SemanticAnalysis::visit(StopStmt& node) {
    const log::Span span = { m_rib->path(), node.span() };

    // Check that stop statements are inside loop bodies.
    if (m_loop == None)
        log::error("'stop' statement outside of loop", span);
}

void SemanticAnalysis::visit(UntilStmt& node) {
    const log::Span span = { m_rib->path(), node.span() };

    Expr* cond = node.condition();
    cond->accept(*this);

    // Check that the while condition can be evaluated to a boolean.
    if (!is_boolean_evaluable(cond->type()))
        log::error("'until' condition must be a boolean", span);

    if (!node.has_body())
        return;

    Loop prev_loop = m_loop;

    m_loop = Until;
    node.body()->accept(*this);

    m_loop = prev_loop;
}

void SemanticAnalysis::visit(BinaryOp& node) {
    VisitorBase::visit(node);

    const log::Span span = { m_rib->path(), node.span() };

    Expr* lhs = node.lhs();
    Expr* rhs = node.rhs();
    Type* lhs_type = lhs->type();
    Type* rhs_type = rhs->type();

    BinaryOp::Operator op = node.op();
    bool supports_ptr_arith = op == BinaryOp::Add || op == BinaryOp::Sub;

    const TypeCheckMode mode = supports_ptr_arith ? Loose : AllowImplicit;
    const TypeCheckResult res = type_check(rhs_type, lhs_type, mode);
    if (res == TypeCheckResult::Cast) {
        node.m_rhs = CastExpr::create(
            *m_rib, 
            rhs->span(), 
            lhs_type, 
            rhs
        );
    } else if (res == TypeCheckResult::Mismatch) {
        log::error("operand type mismatch; got '" + rhs_type->string() + "', but expected '" + lhs_type->string() + "'", span);
    }

    // Set the resulting type of the operator to a 'bool' if the operator is
    // a boolean comparison.
    if (BinaryOp::is_comparison(op)) {
        node.set_type(BuiltinType::get(*m_rib, BuiltinType::Kind::Bool));
        return;
    } else {
        // Default the type of the operator to the LHS type.
        node.set_type(lhs->type());
    }

    // Check that left hand operands of assignments are lvalues.
    if (BinaryOp::is_assignment(op) && !lhs->is_lvalue())
        log::error("left hand operand must be an lvalue", span);
}

void SemanticAnalysis::visit(UnaryOp& node) {
    VisitorBase::visit(node);

    node.set_type(node.expr()->type());

    const log::Span span = { m_rib->path(), node.span() };

    Expr* expr = node.expr();
    Type* type = expr->type();

    switch (node.op()) 
    {
    case UnaryOp::Negate:
        // Check operator type compatibility (numerics only).
        if (!(type->is_integer() || type->is_floating_point()))
            log::error("'-' operator incompatible with '" + type->string() + "'", span);

        node.set_type(type);
        break;

    case UnaryOp::Not:
        // Check operator type compatibility (integers only).
        if (!type->is_integer())
            log::error("'~' operator incompatible with '" + type->string() + "'", span);

        node.set_type(type);
        break;

    case UnaryOp::LogicNot:
        // Check operator type compatibility (scalar only).
        if (!type->is_integer() && !type->is_floating_point() && !dynamic_cast<const PointerType*>(type))
            log::fatal("'!' operator incompatible with '" + type->string() + "'", span);

        node.set_type(type);
        break;

    case UnaryOp::AddressOf: {
        if (!node.expr()->is_lvalue())
            log::fatal("'&' base must be an lvalue", span);

        node.set_type(PointerType::get(*m_rib, type));
        break;
    }

    case UnaryOp::Dereference: {
        PointerType* pt = dynamic_cast<PointerType*>(type);
        if (!pt)
            log::error("'*' operator incompatible with '" + type->string() + "'", span);

        node.set_type(pt->pointee());
        break;
    }

    case UnaryOp::Unknown:
        log::error("unknown unary operator", span);
    }
}

void SemanticAnalysis::visit(CastExpr& node) {
    VisitorBase::visit(node);

    const log::Span span = { m_rib->path(), node.span() };

    Type* castee = node.expr()->type();
    Type* target = node.type();

    if (!castee->can_cast(target))
        log::error("unsupported cast: '" + castee->string() + "' to '" + target->string() + "'", span);
}

void SemanticAnalysis::visit(FieldInitExpr& node) {
    VisitorBase::visit(node);

    node.set_type(node.expr()->type());
}

void SemanticAnalysis::visit(ParenExpr& node) {
    VisitorBase::visit(node);

    node.set_type(node.expr()->type());
}

void SemanticAnalysis::visit(AccessExpr& node) {
    VisitorBase::visit(node);

    const log::Span span = { m_rib->path(), node.span() };

    Type* bt = node.base()->type();
    if (PointerType* ptr = dynamic_cast<PointerType*>(bt))
        bt = ptr->pointee();

    StructType* st = dynamic_cast<StructType*>(bt);
    if (!st) {
        log::error("'.' base must be a struct or a pointer to one; got '" + node.base()->type()->string() + "'", span);
        return;
    }

    StructDefn* sd = st->defn();
    assert(sd);

    FieldDefn* field = sd->get_field(node.name());
    if (field) {
        node.set_field(field);
        node.set_type(field->type());
        return;
    }

    FunctionDefn* method = sd->get_method(node.name());
    if (method) {
        node.set_field(method);
        node.set_type(method->type());
        return;
    }
    
    log::error("no field or method '" + node.name() + "' in '" + sd->name() + "'", span);  
}

void SemanticAnalysis::visit(RefExpr& node) {
    VisitorBase::visit(node);

    ValueDefn* vd = node.defn();
    assert(vd);

    node.set_type(vd->type());
}

void SemanticAnalysis::visit(CallExpr& node) {
    Expr* callee = node.callee();
    callee->accept(*this);

    const log::Span span = { m_rib->path(), node.span() };

    FunctionType* ft = dynamic_cast<FunctionType*>(node.callee()->type());
    if (!ft) {
        log::error("callee is not a valid function", span);
        return;
    }

    // Call expression type is the result type of the callee.
    node.set_type(ft->result());

    // Check that the number of call arguments are the same as the number of
    // expected parameters.
    uint32_t num_args = node.num_args();
    uint32_t num_params = ft->num_params();

    // If the call has a receiver, i.e. is a method call, so there is 
    // technically an extra argument.
    if (node.has_receiver())
        num_args += 1;

    if (num_args != num_params) {
        log::error("argument count mismatch, expected " + std::to_string(num_params) + ", got " + std::to_string(num_args), span);
        return;
    }

    if (node.has_receiver()) {
        // Check that the receiver has the same type as the first argument.
        Expr* receiver = node.receiver();
        assert(receiver);

        Type* expected = ft->get_param(0);
        assert(dynamic_cast<PointerType*>(expected));

        // The type of the receiver must either be 'expected' or '*expected'.
        TypeCheckResult res = type_check(receiver->type(), expected);
        if (res != Match) {
            // The receiver on the function is always a pointer ex. *T, but the
            // receiver on the call didn't match. If the call receiver had type
            // T, then we can wrap it in a pointer and double check.

            PointerType* pt = PointerType::get(*m_rib, receiver->type());

            res = type_check(pt, expected);
            if (res != Match) {
                log::error("receiver type mismatch; got '" + receiver->type()->string() + "'", span);
            }
        }
    }

    // Pass over each argument and compare its type to the functions expected
    // parameter type.
    for (uint32_t i = 0, e = node.num_args(); i < e; ++i) {
        Expr* arg = node.get_arg(i);
        arg->accept(*this);

        Type* actual = arg->type();
        Type* expected;

        if (node.has_receiver()) {
            expected = ft->get_param(i + 1);
        } else {
            expected = ft->get_param(i);
        }

        const TypeCheckResult res = type_check(actual, expected);
        if (res == TypeCheckResult::Cast) {
            node.m_args[i] = CastExpr::create(
                *m_rib, 
                arg->span(), 
                expected, 
                arg
            );
        } else if (res == TypeCheckResult::Mismatch) {
            log::error("argument type mismatch; got '" + actual->string() + "', but expected '" + expected->string() + "'", span);
        }
    }
}

void SemanticAnalysis::visit(StructInitExpr& node) {
    VisitorBase::visit(node);

    const log::Span span = { m_rib->path(), node.span() };

    for (uint32_t i = 0; i < node.num_fields(); ++i) {
        FieldInitExpr* fi = node.get_field(i);
        FieldDefn* field = fi->field();
        assert(field);

        // Check for duplicate fields.
        for (uint32_t j = i + 1; j < node.num_fields(); ++j) {
            if (node.get_field(i)->name() == fi->name()) {
                log::error("struct field initialized more than once: '" + fi->name() + "'", span);
                return;
            }
        }

        Type* actual = fi->type();
        Type* expected = field->type();

        const TypeCheckResult res = type_check(actual, expected);
        if (res == TypeCheckResult::Cast) {
            fi->m_expr = CastExpr::create(
                *m_rib, 
                fi->expr()->span(), 
                expected,
                fi->expr()
            );
        } else if (res == TypeCheckResult::Mismatch) {
            log::error("argument type mismatch; got '" + actual->string() + "', but expected '" + expected->string() + "'", span);
        }
    }
}

void SemanticAnalysis::visit(SubscriptExpr& node) {
    VisitorBase::visit(node);

    const log::Span span = { m_rib->path(), node.span() };

    Expr* base = node.base();
    Expr* index = node.index();

    if (PointerType* pt = dynamic_cast<PointerType*>(base->type())) {
        node.set_type(pt->pointee());
    } else {
        log::error("'[]' operator incompatible with '" + base->type()->string() + "'", span);
    }
}
