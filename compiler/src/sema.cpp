#include "ast.hpp"
#include "logger.hpp"
#include "type.hpp"
#include "visitor.hpp"

using namespace stm;

enum class TypeCheckResult : u8 {
    Success,
    ImplicitCast,
    Failure,
};

enum class TypeCheckMode : u8 {
    Exact,
    AllowImplicit,
    Loose,  
};

static TypeCheckResult type_check(
        const Type* pActual, const Type* pExpected, TypeCheckMode mode) {
    return TypeCheckResult::Success;
}

void SemanticAnalysis::visit(Root& node) {
    for (auto decl : node.decls)
        decl->accept(*this);
}

void SemanticAnalysis::visit(FunctionDecl& node) {
    pFunction = &node;

    if (node.has_body())
        node.pBody->accept(*this);

    pFunction = nullptr;
}

void SemanticAnalysis::visit(VariableDecl& node) {
    if (node.has_init()) {
        node.pInit->accept(*this);

        TypeCheckResult tc = type_check(
            node.get_init()->get_type(), 
            node.get_type(), 
            TypeCheckMode::AllowImplicit);

        if (tc == TypeCheckResult::ImplicitCast) {
            node.pInit = new CastExpr(
                node.get_init()->get_span(),
                node.get_type(),
                Expr::ValueKind::RValue,
                node.pInit);
        } else if (tc == TypeCheckResult::Failure) {
            Logger::fatal("variable type mismatch", node.span);
        }
    }
}

void SemanticAnalysis::visit(BlockStmt& node) {
    for (auto stmt : node.stmts)
        stmt->accept(*this);
}

void SemanticAnalysis::visit(BreakStmt& node) {
    if (loop == Loop::None)
        Logger::fatal("'break' statement outside of loop.", node.span);
}

void SemanticAnalysis::visit(ContinueStmt& node) {
    if (loop == Loop::None)
        Logger::fatal("'continue' statement outside of loop.", node.span);
}

void SemanticAnalysis::visit(DeclStmt& node) {
    node.pDecl->accept(*this);
}

void SemanticAnalysis::visit(IfStmt& node) {
    node.pCond->accept(*this);

    if (dynamic_cast<DeclStmt*>(node.pThen))
        Logger::fatal("declaration must be scoped.", node.pThen->span);

    node.pThen->accept(*this);

    if (node.has_else()) {
        if (dynamic_cast<DeclStmt*>(node.pElse))
            Logger::fatal("declaration must be scoped.", node.pElse->span);

        node.pElse->accept(*this);
    }
}

void SemanticAnalysis::visit(WhileStmt& node) {
    node.pCond->accept(*this);

    if (dynamic_cast<DeclStmt*>(node.pBody))
        Logger::fatal("declaration must be scoped.", node.pBody->get_span());

    Loop prev = loop;
    loop = Loop::While;
    node.pBody->accept(*this);
    loop = prev;
}

void SemanticAnalysis::visit(RetStmt& node) {
    if (!pFunction)
        Logger::fatal("'ret' statement outside of function.", node.span);

    if (node.has_expr()) {
        node.pExpr->accept(*this);

        TypeCheckResult tc = type_check(
            pFunction->get_type(), 
            node.get_expr()->get_type(), 
            TypeCheckMode::AllowImplicit);

        if (tc == TypeCheckResult::ImplicitCast) {
            node.pExpr = new CastExpr(
                node.get_expr()->get_span(),
                pFunction->get_type()->get_return_type(),
                Expr::ValueKind::RValue,
                node.pExpr);
        } else if (tc == TypeCheckResult::Failure) {
            Logger::fatal("return type mismatch", node.span);
        }
    } else if (!pFunction->get_type()->get_return_type()->is_void()) {
        Logger::fatal("function '" + pFunction->name + "' does not return void.", node.span);
    }
}

void SemanticAnalysis::visit(Rune& node) {

}

void SemanticAnalysis::visit(BinaryExpr& node) {
    node.pLeft->accept(*this);
    node.pRight->accept(*this);

    auto left_type = node.get_lhs()->get_type();
    auto right_type = node.get_rhs()->get_type();

    TypeCheckMode mode = TypeCheckMode::AllowImplicit;
    if (BinaryExpr::supports_ptr_arith(node.op))
        mode = TypeCheckMode::Loose;

    TypeCheckResult tc = type_check(
        right_type, left_type, mode);

    if (tc == TypeCheckResult::ImplicitCast) {
        node.pRight = new CastExpr(
            node.get_rhs()->get_span(),
            left_type,
            Expr::ValueKind::RValue,
            node.pRight);
    } else if (tc == TypeCheckResult::Failure) {
        Logger::fatal("binary operand type mismatch", node.span);
    }

    if (BinaryExpr::is_comparison(node.op)) {
        node.pType = BuiltinType::get(root, BuiltinType::Kind::Bool);
        return;
    }

    node.pType = left_type;

    if (BinaryExpr::is_assignment(node.op) 
            && node.get_lhs()->get_value_kind() != Expr::ValueKind::LValue) {
        Logger::fatal("cannot assign to non-lvalue left operand.", node.span);
    }
}

void SemanticAnalysis::visit(UnaryExpr& node) {
    node.pExpr->accept(*this);
}

void SemanticAnalysis::visit(CastExpr& node) {
    node.pExpr->accept(*this);

    // if nested expr cannot cast to cast type, fatal
}

void SemanticAnalysis::visit(ParenExpr& node) {
    node.pExpr->accept(*this);
}

void SemanticAnalysis::visit(SizeofExpr& node) {

}

void SemanticAnalysis::visit(SubscriptExpr& node) {
    node.pBase->accept(*this);
    node.pIndex->accept(*this);
}

void SemanticAnalysis::visit(ReferenceExpr& node) {

}

void SemanticAnalysis::visit(MemberExpr& node) {

}

void SemanticAnalysis::visit(CallExpr& node) {

}

void SemanticAnalysis::visit(RuneExpr& node) {

}
