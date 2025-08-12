#include "ast.hpp"
#include "type.hpp"

#include <cassert>

using namespace stm;

const BuiltinType* TypeContext::get(BuiltinType::Kind kind) const {
    return builtins.at(kind);
}

const DeferredType* TypeContext::get(const DeferredType::Context& context) {
    DeferredType* type = new DeferredType(context);
    deferred.push_back(type);
    return type;
}

const FunctionType* TypeContext::get(const Type* pReturn, const std::vector<const Type*> &params) {
    FunctionType* type = new FunctionType(pReturn, params);
    functions.push_back(type);
    return type;
}

const PointerType* TypeContext::get(const Type* pPointee) {
    auto it = pointers.find(pPointee);
    if (it != pointers.end())
        return it->second;

    PointerType* type = new PointerType(pPointee);
    pointers.emplace(pPointee, type);
    return type;
}

TypeContext::TypeContext() {
    for (auto kind = BuiltinType::Kind::Void; 
          kind != BuiltinType::Kind::Float64; 
          kind = BuiltinType::Kind(u8(kind) + 1)) {
        builtins.emplace(kind, new BuiltinType(kind));
    }
}

TypeContext::~TypeContext() {
    for (auto [ kind, type ] : builtins)
        delete type;

    for (auto& type : deferred)
        delete type;

    for (auto& type : functions)
        delete type;

    for (auto [ pointee, type ] : pointers)
        delete type;

    builtins.clear();
    deferred.clear();
    functions.clear();
    pointers.clear();
}

Root::Root(InputFile& file, Scope* pScope, const std::vector<Decl*>& decls)
    : file(file), context(), pScope(pScope), decls(decls), imports(), exports() {};

Root::~Root() {
    delete pScope;
    pScope = nullptr;

    for (auto decl : decls)
        delete decl;

    decls.clear();
    imports.clear();
    exports.clear();
}

Decl::Decl(const Span& span, const std::string& name, const std::vector<Rune*>& decorators)
    : span(span), name(name), decorators(decorators) {};

FunctionDecl::FunctionDecl(
        const Span& span, 
        const std::string& name, 
        const std::vector<Rune*>& decorators, 
        const FunctionType* pType, 
        const std::vector<ParameterDecl*>& params,
        Scope* pScope,
        Stmt* pBody)
    : Decl(span, name, decorators), pType(pType), params(params), pScope(pScope), pBody(pBody) {};

FunctionDecl::~FunctionDecl() {
    for (auto param : params)
        delete param;

    params.clear();
    
    delete pScope;
    pScope = nullptr;

    delete pBody;
    pBody = nullptr;
}

ParameterDecl::ParameterDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& decorators,
        const Type* pType)
    : Decl(span, name, decorators), pType(pType) {};

VariableDecl::VariableDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& decorators,
        const Type* pType)
    : Decl(span, name, decorators), pType(pType) {};

VariableDecl::~VariableDecl() {
    if (pInit != nullptr) {
        delete pInit;
        pInit = nullptr;
    }
}

BlockStmt::BlockStmt(
        const Span& span, 
        const std::vector<Rune*>& runes, 
        const std::vector<Stmt*>& stmts, 
        Scope* pScope)
    : Stmt(span), runes(runes), stmts(stmts), pScope(pScope) {};

BlockStmt::~BlockStmt() {
    for (Rune* rune : runes)
        delete rune;

    for (Stmt* stmt : stmts)
        delete stmt;

    delete pScope;
    pScope = nullptr;

    runes.clear();
    stmts.clear();
}

DeclStmt::DeclStmt(const Span& span, Decl* pDecl) : Stmt(span), pDecl(pDecl) {};

DeclStmt::~DeclStmt() {
    if (pDecl != nullptr) {
        delete pDecl;
        pDecl = nullptr;
    }
}

IfStmt::IfStmt(const Span& span, Expr* pCond, Stmt* pThen, Stmt* pElse)
    : Stmt(span), pCond(pCond), pThen(pThen), pElse(pElse) {};

IfStmt::~IfStmt() {
    if (pCond != nullptr) {
        delete pCond;
        pCond = nullptr;
    }

    if (pThen != nullptr) {
        delete pThen;
        pThen = nullptr;
    }

    if (pElse != nullptr) {
        delete pElse;
        pElse = nullptr;
    }
}

WhileStmt::WhileStmt(const Span& span, Expr* pCond, Stmt* pBody) 
    : Stmt(span), pCond(pCond), pBody(pBody) {};

WhileStmt::~WhileStmt() {
    if (pCond != nullptr) {
        delete pCond;
        pCond = nullptr;
    }

    if (pBody != nullptr) {
        delete pBody;
        pBody = nullptr;
    }
}

RetStmt::RetStmt(const Span& span, Expr* pExpr) : Stmt(span), pExpr(pExpr) {};

RetStmt::~RetStmt() {
    if (pExpr != nullptr) {
        delete pExpr;
        pExpr = nullptr;
    }
}

Rune::Rune(const Span& span, Kind kind, const std::vector<Expr*>& args) 
    : Stmt(span), kind(kind), args(args) {};

Rune::~Rune() {
    for (Expr* arg : args)
        delete arg;

    args.clear();
}

Expr::Expr(const Span& span, const Type* pType, ValueKind vkind) 
    : Stmt(span), pType(pType), vkind(vkind) {};

BinaryExpr::BinaryExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        Operator op, 
        Expr* pLeft, 
        Expr* pRight)
    : Expr(span, pType, ValueKind::RValue), op(op), pLeft(pLeft), 
        pRight(pRight) {};

BinaryExpr::~BinaryExpr() {
    if (pLeft != nullptr) {
        delete pLeft;
        pLeft = nullptr;
    }

    if (pRight != nullptr) {
        delete pRight;
        pRight = nullptr;
    }
}

UnaryExpr::UnaryExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        Operator op, 
        Expr* pExpr, 
        bool postfix)
    : Expr(span, pType, vkind), op(op), pExpr(pExpr), postfix(postfix) {};
    
UnaryExpr::~UnaryExpr() {
    if (pExpr != nullptr) {
        delete pExpr;
        pExpr = nullptr;
    }
}

bool UnaryExpr::is_constant() const {
    if (op == Operator::Address_Of)
        return true;
    
    return pExpr->is_constant();
}

CastExpr::CastExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        Expr* pExpr)
    : Expr(span, pType, vkind), pExpr(pExpr) {};

CastExpr::~CastExpr() {
    if (pExpr != nullptr) {
        delete pExpr;
        pExpr = nullptr;
    }
}

ParenExpr::ParenExpr(
        const Span& span,
        Expr* pExpr)
    : Expr(span, pExpr->get_type(), pExpr->get_value_kind()), pExpr(pExpr) {};

ParenExpr::~ParenExpr() {
    if (pExpr != nullptr) {
        delete pExpr;
        pExpr = nullptr;
    }
}

SizeofExpr::SizeofExpr(
        const Span& span,
        const Type* pType,
        const Type* pTarget)
    : Expr(span, pType, ValueKind::RValue), pTarget(pTarget) {};

SubscriptExpr::SubscriptExpr(
        const Span& span, 
        const Type* pType,
        ValueKind vkind, 
        Expr* pBase, 
        Expr* pIndex)
    : Expr(span, pType, vkind), pBase(pBase), pIndex(pIndex) {};

SubscriptExpr::~SubscriptExpr() {
    if (pBase != nullptr) {
        delete pBase;
        pBase = nullptr;
    }

    if (pIndex != nullptr) {
        delete pIndex;
        pIndex = nullptr;
    }
}

ReferenceExpr::ReferenceExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        const std::string& ref)
    : Expr(span, pType, vkind), ref(ref) {};

MemberExpr::MemberExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        const std::string& member, 
        Expr* pBase)
    : ReferenceExpr(span, pType, vkind, member), pBase(pBase) {};

MemberExpr::~MemberExpr() {
    if (pBase != nullptr) {
        delete pBase;
        pBase = nullptr;
    }
}

CallExpr::CallExpr(
        const Span& span, 
        const Type* pType, 
        const std::string& callee, 
        const std::vector<Expr*> &args)
    : ReferenceExpr(span, pType, ValueKind::RValue, callee), args(args) {};

CallExpr::~CallExpr() {
    for (Expr* arg : args)
        delete arg;

    args.clear();
}

RuneExpr::RuneExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        Rune* pRune)
    : Expr(span, pType, vkind), pRune(pRune) {};

RuneExpr::~RuneExpr() {
    if (pRune != nullptr) {
        delete pRune;
        pRune = nullptr;
    }
}

bool RuneExpr::is_constant() const {
    switch (pRune->get_kind()) {
        case Rune::Kind::Code:
        case Rune::Kind::Comptime:
        case Rune::Kind::Path:
            return true;
        default:
            assert(false && "cannot use rune as value expression");
    }
}
