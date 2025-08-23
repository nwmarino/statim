#include "tree/rune.hpp"

stm::Rune::Rune(const Span& span, Kind kind, const std::vector<Expr*>& args) 
    : Stmt(span), kind(kind), args(args) {};

stm::Rune::~Rune() {
    for (Expr* arg : args)
        delete arg;

    args.clear();
}

stm::RuneExpr::RuneExpr(
        const Span& span, 
        const Type* pType, 
        Rune* pRune)
    : Expr(span, pType), pRune(pRune) {};

stm::RuneExpr::~RuneExpr() {
    if (pRune != nullptr) {
        delete pRune;
        pRune = nullptr;
    }
}

bool stm::RuneExpr::is_constant() const {
    switch (pRune->get_kind()) {
    case Rune::Kind::Code:
    case Rune::Kind::Comptime:
    case Rune::Kind::Path:
        return true;
    default:
        assert(false && "cannot use rune as value expression");
    }
}
