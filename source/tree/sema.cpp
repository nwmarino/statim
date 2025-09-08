#include "core/logger.hpp"
#include "tree/type.hpp"
#include "tree/decl.hpp"
#include "tree/expr.hpp"
#include "tree/root.hpp"
#include "tree/rune.hpp"
#include "tree/stmt.hpp"

#include "tree/visitor.hpp"
#include <string>

using namespace stm;

/// Different type-checking mode severities.
enum class TypeCheckMode : u8 {
    Exact, AllowImplicit, Loose
};

/// Different results of type-checking.
enum class TypeCheckResult : u8 {
    Mismatch, Match, Cast
};

/// Perform a type check between a given type and the expected type.
/// \returns The result of the check, either a match, mismatch, or if an
/// implicit cast should be injected at the site of the given typed node.
static TypeCheckResult type_check(
        const Type* pActual, 
        const Type* pExpected, 
        TypeCheckMode mode) {
    if (auto actual_deferred = dynamic_cast<const DeferredType*>(pActual))
        return type_check(actual_deferred->get_resolved(), pExpected, mode);
    else if (auto expected_deferred = dynamic_cast<const DeferredType*>(pExpected))
        return type_check(pActual, expected_deferred->get_resolved(), mode);
    
    if (*pActual == *pExpected)
        return TypeCheckResult::Match;

    switch (mode) {
    case TypeCheckMode::Exact:
        // The types already failed the strict comparison, so it's a mismatch.
        return TypeCheckResult::Mismatch;

    case TypeCheckMode::AllowImplicit:
        // If we can cast the actual type to the desired type, then respond
        // with a cast injection.
        if (pActual->can_cast(pExpected, true))
            return TypeCheckResult::Cast;

        return TypeCheckResult::Mismatch;
        
    case TypeCheckMode::Loose: {
        if (pActual->can_cast(pExpected, true))
            return TypeCheckResult::Cast;
        
        // Since pointer -> int and int -> pointer casts cannot be done 
        // implicitly, this loose matching allows for it under rare 
        // circumstances like in pointer arithmetic.
        if (dynamic_cast<const PointerType*>(pActual) && pExpected->is_int())
            return TypeCheckResult::Match;
        else if (dynamic_cast<const PointerType*>(pActual) && dynamic_cast<const PointerType*>(pExpected))
            return TypeCheckResult::Match;
        else if (pActual->is_int() && dynamic_cast<const PointerType*>(pExpected))
            return TypeCheckResult::Match;

        return TypeCheckResult::Mismatch;
    }
    
    }
    
    return TypeCheckResult::Mismatch;
}

void SemanticAnalysis::visit(Root& node) {
    for (auto decl : node.decls) decl->accept(*this);
}

void SemanticAnalysis::visit(FunctionDecl& node) {
    pFunction = &node;

    /// Function signature checking for 'main' function.
    /// TODO: Improve this to unify errors with the expected signature.
    if (node.get_name() == "main") {
        const FunctionType* type = node.get_type();

        const Type* return_type = type->get_return_type();
        if (return_type->is_deferred())
            return_type = return_type->as_deferred()->get_resolved();
        
        if (*return_type != *root.get_si64_type()) {
            Logger::fatal(
                "'main' function should return 's64' type, got '" + 
                    type->get_return_type()->to_string() + "' instead",
                node.get_span());
        }

        if (node.num_params() != 2) {
            Logger::fatal(
                "'main' function should have two parameters, got " + 
                    std::to_string(node.num_params()) + " instead",
                node.get_span());
        }

        const Type* param1_type = node.get_param(0)->get_type();
        if (param1_type->is_deferred())
            param1_type = param1_type->as_deferred()->get_resolved();

        if (*param1_type != *root.get_si64_type()) {
            Logger::fatal(
                "'main' function first parameter should have 's64' type, got '"
                    + param1_type->to_string() + "' instead",
                node.get_param(0)->get_span());
        }

        const Type* param2_type = node.get_param(1)->get_type();
        if (param2_type->is_deferred())
            param2_type = param2_type->as_deferred()->get_resolved();

        bool param2_adequate = true;

        // Check that the parameter type is *...
        if (param2_type->is_pointer()) {
            const Type* pointee = param2_type->as_pointer()->get_pointee();
            if (pointee->is_deferred())
                pointee = pointee->as_deferred()->get_resolved();

            // Check that the parameter type is **...
            if (pointee->is_pointer()) {
                const Type* base = pointee->as_pointer()->get_pointee();
                if (base->is_deferred())
                    base = base->as_deferred()->get_resolved();

                // Check that the parameter type is **char
                if (*base != *root.get_char_type())
                    param2_adequate = false;
            } else {
                param2_adequate = false;
            }
        } else {
            param2_adequate = false;
        }

        if (!param2_adequate) {
            Logger::fatal(
                "'main' function second parameter should have '**char' type, got '"
                    + param2_type->to_string() + "' instead",
                node.get_param(1)->get_span());
        }
    }

    if (node.has_body()) 
        node.pBody->accept(*this);
    
    pFunction = nullptr;
}

void SemanticAnalysis::visit(VariableDecl& node) {
    if (node.has_init()) {
        node.pInit->accept(*this);

        // Perform a type check to try and match the type of the initializer
        // to the type presented in the variable declaration.
        TypeCheckResult tc = type_check(
            node.get_init()->get_type(), 
            node.get_type(), 
            TypeCheckMode::AllowImplicit);

        if (tc == TypeCheckResult::Cast) {
            node.pInit = new CastExpr(
                node.get_init()->get_span(),
                node.get_type(),
                node.pInit);
        } else if (tc == TypeCheckResult::Mismatch) {
            Logger::fatal(
                "variable type mismatch, got '" + 
                    node.get_init()->get_type()->to_string() + 
                    "', but expected '" + node.get_type()->to_string() + "'", 
                node.get_span());
        }
    }
}

void SemanticAnalysis::visit(BlockStmt& node) {
    for (auto stmt : node.stmts) stmt->accept(*this);
}

void SemanticAnalysis::visit(BreakStmt& node) {
    if (loop == Loop::None) {
        Logger::fatal(
            "'break' statement outside of loop", 
            node.get_span());
    }
}

void SemanticAnalysis::visit(ContinueStmt& node) {
    if (loop == Loop::None) {
        Logger::fatal(
            "'continue' statement outside of loop", 
            node.get_span());
    }
}

void SemanticAnalysis::visit(DeclStmt& node) {
    node.pDecl->accept(*this);
}

void SemanticAnalysis::visit(IfStmt& node) {
    node.pCond->accept(*this);

    if (!node.get_cond()->get_type()->is_bool()) {
        Logger::fatal(
            "'if' condition must be a boolean",
            node.get_cond()->get_span());
    }

    if (dynamic_cast<const DeclStmt*>(node.get_then())) {
        Logger::fatal(
            "declaration must be within a block statement", 
            node.get_then()->get_span());
    }

    node.pThen->accept(*this);

    if (node.has_else()) {
        if (dynamic_cast<const DeclStmt*>(node.get_else())) {
            Logger::fatal(
                "declaration must be witin a block statement", 
                node.get_else()->get_span());
        }

        node.pElse->accept(*this);
    }
}

void SemanticAnalysis::visit(WhileStmt& node) {
    node.pCond->accept(*this);

    if (!node.get_cond()->get_type()->is_bool()) {
        Logger::fatal(
            "'while' condition must be a boolean",
            node.get_cond()->get_span());
    }

    if (dynamic_cast<const DeclStmt*>(node.get_body())) {
        Logger::fatal(
            "declaration must be within a block statement", 
            node.get_body()->get_span());
    }
    
    Loop prev = loop;
    loop = Loop::While;
    node.pBody->accept(*this);
    loop = prev;
}

void SemanticAnalysis::visit(RetStmt& node) {
    if (!pFunction) {
        Logger::fatal(
            "'ret' statement outside of function", 
            node.get_span());
    }

    if (node.has_expr()) {
        node.pExpr->accept(*this);

        // Perform a type check between the type of the return expression and
        // the function return type.
        TypeCheckResult tc = type_check(
            pFunction->get_type()->get_return_type(), 
            node.get_expr()->get_type(), 
            TypeCheckMode::AllowImplicit);

        if (tc == TypeCheckResult::Cast) {
            node.pExpr = new CastExpr(
                node.get_expr()->get_span(),
                pFunction->get_type()->get_return_type(),
                node.pExpr);
        } else if (tc == TypeCheckResult::Mismatch) {
            Logger::fatal(
                "function return type mismatch, got '" + 
                    node.get_expr()->get_type()->to_string() + 
                    "', but expected '" + 
                    pFunction->get_type()->get_return_type()->to_string() + "'", 
                node.get_span());
        }
    } else if (!pFunction->get_type()->get_return_type()->is_void()) {
        Logger::fatal(
            "return statement is empty, but function '" + 
                pFunction->get_name() + "' does not return void", 
            node.get_span());
    }
}

void SemanticAnalysis::visit(BinaryExpr& node) {
    node.pLeft->accept(*this);
    node.pRight->accept(*this);

    auto left_type = node.get_lhs()->get_type();
    auto right_type = node.get_rhs()->get_type();

    TypeCheckMode mode = TypeCheckMode::AllowImplicit;
    if (BinaryExpr::supports_ptr_arith(node.op)) {
        // Since pointer arithmetic involves integers and pointers, but they
        // cannot be implicitly casted to one another, looser type checking is
        // necessary.
        mode = TypeCheckMode::Loose;
    }

    // Perform a type check between the two operands of the binary operator.
    TypeCheckResult tc = type_check(
        right_type, left_type, mode);

    if (tc == TypeCheckResult::Cast) {
        node.pRight = new CastExpr(
            node.get_rhs()->get_span(),
            left_type,
            node.pRight);
    } else if (tc == TypeCheckResult::Mismatch) {
        Logger::fatal(
            "binary operand type mismatch, left side has type '" + 
                left_type->to_string() + "', but right side is '" + 
                right_type->to_string() + "'", 
            node.get_span());
    }

    if (BinaryExpr::is_comparison(node.get_operator())) {
        // The result of a comparison is always a boolean.
        node.pType = BuiltinType::get(root, BuiltinType::Kind::Bool);
        return;
    }

    // Propogate the type of the binary expression to the be the type of the
    // left hand side at this point.
    node.pType = left_type;

    if (BinaryExpr::is_assignment(node.get_operator()) 
            && !node.get_lhs()->is_lvalue()) {
        // Ensure that assignment operators only assign to lvalues.
        Logger::fatal(
            "cannot assign to non-lvalue left operand", 
            node.get_span());
    }

    /// TODO: Perform mutability checks for assignment operators.
}

void SemanticAnalysis::visit(UnaryExpr& node) {
    node.pExpr->accept(*this);
}

void SemanticAnalysis::visit(CastExpr& node) {
    node.pExpr->accept(*this);

    const Type* expr_type = node.get_expr()->get_type();
    const Type* cast_type = node.get_type();

    // if nested expr cannot cast to cast type, fatal
    if (!expr_type->can_cast(cast_type)) Logger::fatal(
        "cannot cast type '" + expr_type->to_string() + "' to '" + 
            cast_type->to_string() + "'",
        node.get_span());
}

void SemanticAnalysis::visit(ParenExpr& node) {
    node.pExpr->accept(*this);
    node.pType = node.get_expr()->get_type();
}

void SemanticAnalysis::visit(SubscriptExpr& node) {
    node.pBase->accept(*this);
    node.pIndex->accept(*this);
}

void SemanticAnalysis::visit(MemberExpr& node) {
    /// TODO: Perform visibility checks based on field runes.
}

void SemanticAnalysis::visit(CallExpr& node) {
    auto callee = static_cast<const FunctionDecl*>(node.get_decl());

    for (u32 idx = 0, e = node.num_args(); idx != e; ++idx) {
        auto arg = node.args[idx];
        auto param = callee->get_param(idx);

        arg->accept(*this);

        // Perform a type check between the type of the call argument and the
        // type of the corresponding parameter.
        TypeCheckResult tc = type_check(
            arg->get_type(), 
            param->get_type(), 
            TypeCheckMode::AllowImplicit);

        if (tc == TypeCheckResult::Cast) {
            node.args[idx] = new CastExpr(
                arg->get_span(),
                param->get_type(),
                arg);
        } else if (tc == TypeCheckResult::Mismatch) {
            Logger::fatal(
                "call argument type mismatch, got '" + 
                    arg->get_type()->to_string() + "', but expected '" + 
                    param->get_type()->to_string() + "'", 
                node.get_span());
        }
    }
}

void SemanticAnalysis::visit(RuneExpr& node) {

}

void SemanticAnalysis::visit(RuneStmt& node) {

}
