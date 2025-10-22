#include "tree/expr.hpp"

stm::Expr::Expr(const Span& span, const Type* pType)   
    : Stmt(span), pType(pType) {}

stm::BinaryExpr::BinaryExpr(
        const Span& span, 
        const Type* pType, 
        Operator op, 
        Expr* pLeft, 
        Expr* pRight)
    : Expr(span, pType), op(op), pLeft(pLeft), 
        pRight(pRight) {};

stm::BinaryExpr::~BinaryExpr() {
    if (pLeft != nullptr) {
        delete pLeft;
        pLeft = nullptr;
    }

    if (pRight != nullptr) {
        delete pRight;
        pRight = nullptr;
    }
}

bool stm::BinaryExpr::is_comparison(Operator op) {
    switch (op) {
    case Operator::Equals:
    case Operator::Not_Equals:
    case Operator::Less_Than:
    case Operator::Less_Than_Equals:
    case Operator::Greater_Than:
    case Operator::Greater_Than_Equals:
    case Operator::Logical_And:
    case Operator::Logical_Or:
        return true;
    default:
        return false;
    }
}

bool stm::BinaryExpr::is_logical_comparison(Operator op) {
    switch (op) {
    case Operator::Logical_And:
    case Operator::Logical_Or:
        return true;
    default:
        return false;
    }
}

bool stm::BinaryExpr::is_assignment(Operator op) {
    switch (op) {
    case Operator::Assign:
    case Operator::Add_Assign:
    case Operator::Sub_Assign:
    case Operator::Mul_Assign:
    case Operator::Div_Assign:
    case Operator::Mod_Assign:
    case Operator::Bitwise_And_Assign:
    case Operator::Bitwise_Or_Assign:
    case Operator::Bitwise_Xor_Assign:
    case Operator::Left_Shift_Assign:
    case Operator::Right_Shift_Assign:
        return true;
    default:
        return false;
    }
}

bool stm::BinaryExpr::supports_ptr_arith(Operator op) {
    switch (op) {
    case Operator::Add:
    case Operator::Add_Assign:
    case Operator::Sub:
    case Operator::Sub_Assign:
        return true;
    default:
        return false;
    }
}

stm::UnaryExpr::UnaryExpr(
        const Span& span, 
        const Type* pType, 
        Operator op, 
        Expr* pExpr, 
        bool postfix)
    : Expr(span, pType), op(op), pExpr(pExpr), postfix(postfix) {};
    
stm::UnaryExpr::~UnaryExpr() {
    if (pExpr != nullptr) {
        delete pExpr;
        pExpr = nullptr;
    }
}

bool stm::UnaryExpr::is_constant() const {
    if (op == Operator::Address_Of)
        return true;
    
    return pExpr->is_constant();
}

stm::CastExpr::CastExpr(
        const Span& span, 
        const Type* pType, 
        Expr* pExpr)
    : Expr(span, pType), pExpr(pExpr) {};

stm::CastExpr::~CastExpr() {
    if (pExpr != nullptr) {
        delete pExpr;
        pExpr = nullptr;
    }
}

stm::ParenExpr::ParenExpr(
        const Span& span,
        Expr* pExpr)
    : Expr(span, pExpr->get_type()), pExpr(pExpr) {};

stm::ParenExpr::~ParenExpr() {
    if (pExpr != nullptr) {
        delete pExpr;
        pExpr = nullptr;
    }
}

stm::SizeofExpr::SizeofExpr(
        const Span& span,
        const Type* pType,
        const Type* pTarget)
    : Expr(span, pType), pTarget(pTarget) {};

stm::SubscriptExpr::SubscriptExpr(
        const Span& span, 
        const Type* pType,
        Expr* pBase, 
        Expr* pIndex)
    : Expr(span, pType), pBase(pBase), pIndex(pIndex) {};

stm::SubscriptExpr::~SubscriptExpr() {
    if (pBase != nullptr) {
        delete pBase;
        pBase = nullptr;
    }

    if (pIndex != nullptr) {
        delete pIndex;
        pIndex = nullptr;
    }
}

stm::ReferenceExpr::ReferenceExpr(
        const Span& span, 
        const Type* pType, 
        const std::string& name)
    : Expr(span, pType), name(name) {};

stm::MemberExpr::MemberExpr(
        const Span& span, 
        const Type* pType, 
        const std::string& member, 
        Expr* pBase)
    : ReferenceExpr(span, pType, member), pBase(pBase) {};

stm::MemberExpr::~MemberExpr() {
    if (pBase != nullptr) {
        delete pBase;
        pBase = nullptr;
    }
}

stm::CallExpr::CallExpr(
        const Span& span, 
        const Type* pType, 
        const std::string& callee, 
        const std::vector<Expr*>& args)
    : ReferenceExpr(span, pType, callee), args(args) {};

stm::CallExpr::~CallExpr() {
    for (Expr* arg : args)
        delete arg;

    args.clear();
}
