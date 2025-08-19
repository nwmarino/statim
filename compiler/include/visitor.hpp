#ifndef STATIM_VISITOR_HPP_
#define STATIM_VISITOR_HPP_

#include "bytecode.hpp"
#include "options.hpp"
#include "scope.hpp"
#include "types.hpp"

#include <memory>
#include <optional>

namespace stm {

class TranslationUnit;
class Frame;
class Root;

class Decl;
class FunctionDecl;
class ParameterDecl;
class VariableDecl;
class FieldDecl;
class StructDecl;
class EnumValueDecl;
class EnumDecl;

class Stmt;
class BlockStmt;
class BreakStmt;
class ContinueStmt;
class DeclStmt;
class IfStmt;
class WhileStmt;
class RetStmt;

class Rune;

class Expr;
class BoolLiteral;
class IntegerLiteral;
class FloatLiteral;
class CharLiteral;
class StringLiteral;
class NullLiteral;
class BinaryExpr;
class UnaryExpr;
class CastExpr;
class ParenExpr;
class SizeofExpr;
class SubscriptExpr;
class ReferenceExpr;
class MemberExpr;
class CallExpr;
class RuneExpr;

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void visit(Root& node) = 0;
    
    virtual void visit(FunctionDecl& node) = 0;
    virtual void visit(ParameterDecl& node) = 0;
    virtual void visit(VariableDecl& node) = 0;
    virtual void visit(FieldDecl& node) = 0;
    virtual void visit(StructDecl& node) = 0;
    virtual void visit(EnumValueDecl& node) = 0;
    virtual void visit(EnumDecl& node) = 0;

    virtual void visit(BlockStmt& node) = 0;
    virtual void visit(BreakStmt& node) = 0;
    virtual void visit(ContinueStmt& node) = 0;
    virtual void visit(DeclStmt& node) = 0;
    virtual void visit(IfStmt& node) = 0;
    virtual void visit(WhileStmt& node) = 0;
    virtual void visit(RetStmt& node) = 0;
    virtual void visit(Rune& node) = 0;

    virtual void visit(BoolLiteral& node) = 0;
    virtual void visit(IntegerLiteral& node) = 0;
    virtual void visit(FloatLiteral& node) = 0;
    virtual void visit(CharLiteral& node) = 0;
    virtual void visit(StringLiteral& node) = 0;
    virtual void visit(NullLiteral& node) = 0;
    virtual void visit(BinaryExpr& node) = 0;
    virtual void visit(UnaryExpr& node) = 0;
    virtual void visit(CastExpr& node) = 0;
    virtual void visit(ParenExpr& node) = 0;
    virtual void visit(SizeofExpr& node) = 0;
    virtual void visit(SubscriptExpr& node) = 0;
    virtual void visit(ReferenceExpr& node) = 0;
    virtual void visit(MemberExpr& node) = 0;
    virtual void visit(CallExpr& node) = 0;
    virtual void visit(RuneExpr& node) = 0;
};

class SymbolAnalysis final : public Visitor {
    Options&        opts;
    Root&           root;
    const Scope*    pScope;

public:
    SymbolAnalysis(Options& opts, Root& root);

    void visit(Root& node) override;

    void visit(FunctionDecl& node) override;
    void visit(ParameterDecl& node) override {};
    void visit(VariableDecl& node) override;
    void visit(FieldDecl& node) override;
    void visit(StructDecl& node) override;
    void visit(EnumValueDecl& node) override;
    void visit(EnumDecl& node) override;

    void visit(BlockStmt& node) override;
    void visit(BreakStmt& node) override {};
    void visit(ContinueStmt& node) override {};
    void visit(DeclStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(Rune& node) override;

    void visit(BoolLiteral& node) override {};
    void visit(IntegerLiteral& node) override {};
    void visit(FloatLiteral& node) override {};
    void visit(CharLiteral& node) override {};
    void visit(StringLiteral& node) override {};
    void visit(NullLiteral& node) override {};
    void visit(BinaryExpr& node) override;
    void visit(UnaryExpr& node) override;
    void visit(CastExpr& node) override;
    void visit(ParenExpr& node) override;
    void visit(SizeofExpr& node) override {};
    void visit(SubscriptExpr& node) override;
    void visit(ReferenceExpr& node) override;
    void visit(MemberExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(RuneExpr& node) override;
};

class SemanticAnalysis final : public Visitor {
    enum class Loop : u8 {
        None,
        While,
    };

    Options&        opts;
    Root&           root;
    FunctionDecl*   pFunction = nullptr;
    Loop            loop = Loop::None;

public:
    SemanticAnalysis(Options& opts, Root& root) : opts(opts), root(root) {};

    void visit(Root& node) override;

    void visit(FunctionDecl& node) override;
    void visit(ParameterDecl& node) override {};
    void visit(VariableDecl& node) override;
    void visit(FieldDecl& node) override;
    void visit(StructDecl& node) override;
    void visit(EnumValueDecl& node) override;
    void visit(EnumDecl& node) override;

    void visit(BlockStmt& node) override;
    void visit(BreakStmt& node) override;
    void visit(ContinueStmt& node) override;
    void visit(DeclStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(Rune& node) override;

    void visit(BoolLiteral& node) override {};
    void visit(IntegerLiteral& node) override {};
    void visit(FloatLiteral& node) override {};
    void visit(CharLiteral& node) override {};
    void visit(StringLiteral& node) override {};
    void visit(NullLiteral& node) override {};
    void visit(BinaryExpr& node) override;
    void visit(UnaryExpr& node) override;
    void visit(CastExpr& node) override;
    void visit(ParenExpr& node) override;
    void visit(SizeofExpr& node) override {};
    void visit(SubscriptExpr& node) override;
    void visit(ReferenceExpr& node) override {};
    void visit(MemberExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(RuneExpr& node) override;
};

class Codegen final : public Visitor {
    enum class Phase : u8 {
        Declare, Define,
    };

    enum class ValueContext : u8 {
        LValue, RValue,
    };
    
    Phase                   phase;
    ValueContext            vctx;
    Options&                opts;
    Root&                   root;
    std::unique_ptr<Frame>  frame = nullptr;
    Function*               pFunction = nullptr;
    BasicBlock*             pInsert = nullptr;
    std::optional<Operand>  tmp = std::nullopt;
    u32                     vreg = 1;

    BasicBlock*             pCond = nullptr;
    BasicBlock*             pMerge = nullptr;

public:
    Codegen(Options& opts, Root& root) : opts(opts), root(root) {};

    void run(TranslationUnit& unit);

    void visit(Root& node) override;

    void visit(FunctionDecl& node) override;
    void visit(ParameterDecl& node) override {};
    void visit(VariableDecl& node) override;
    void visit(FieldDecl& node) override;
    void visit(StructDecl& node) override;
    void visit(EnumValueDecl& node) override;
    void visit(EnumDecl& node) override;

    void visit(BlockStmt& node) override;
    void visit(BreakStmt& node) override;
    void visit(ContinueStmt& node) override;
    void visit(DeclStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(Rune& node) override;

    void visit(BoolLiteral& node) override;
    void visit(IntegerLiteral& node) override;
    void visit(FloatLiteral& node) override;
    void visit(CharLiteral& node) override;
    void visit(StringLiteral& node) override;
    void visit(NullLiteral& node) override;
    void visit(BinaryExpr& node) override;
    void visit(UnaryExpr& node) override;
    void visit(CastExpr& node) override;
    void visit(ParenExpr& node) override;
    void visit(SizeofExpr& node) override;
    void visit(SubscriptExpr& node) override;
    void visit(ReferenceExpr& node) override;
    void visit(MemberExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(RuneExpr& node) override;
};

} // namespace stm

#endif // STATIM_VISITOR_HPP_
