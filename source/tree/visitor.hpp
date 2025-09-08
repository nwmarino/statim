#ifndef STATIM_VISITOR_HPP_
#define STATIM_VISITOR_HPP_

#include "siir/basicblock.hpp"
#include "siir/cfg.hpp"
#include "siir/function.hpp"
#include "siir/instbuilder.hpp"
#include "siir/value.hpp"
#include "tree/scope.hpp"
#include "types/options.hpp"
#include "types/types.hpp"

#include <unordered_map>

namespace stm {

class Type;

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
class RuneStmt;

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

/// Abstract visitor pattern over a syntax tree.
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
    virtual void visit(RuneStmt& node) = 0;

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

/// A light-weight resolution pass over a syntax tree to resolve deferred 
/// symbol references such as function calls or those referencing imported 
/// names.
class SymbolAnalysis final : public Visitor {
    Options& opts;
    Root& root;
    const Scope* pScope;

public:
    SymbolAnalysis(Options& opts, Root& root);

    void visit(Root& node) override;

    void visit(FunctionDecl& node) override;
    void visit(ParameterDecl& node) override {};
    void visit(VariableDecl& node) override;
    void visit(FieldDecl& node) override {};
    void visit(StructDecl& node) override {};
    void visit(EnumValueDecl& node) override {};
    void visit(EnumDecl& node) override {};

    void visit(BlockStmt& node) override;
    void visit(BreakStmt& node) override {};
    void visit(ContinueStmt& node) override {};
    void visit(DeclStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(RuneStmt& node) override;

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

/// A semantic pass over a syntax tree to perform language-based validation
/// like type-checking, implicit casting, loop semantics, etc.
class SemanticAnalysis final : public Visitor {
    enum class Loop : u8 { None, While } loop = Loop::None;
    Options& opts;
    Root& root;
    FunctionDecl* pFunction = nullptr;

public:
    SemanticAnalysis(Options& opts, Root& root) : opts(opts), root(root) {};

    void visit(Root& node) override;

    void visit(FunctionDecl& node) override;
    void visit(ParameterDecl& node) override {};
    void visit(VariableDecl& node) override;
    void visit(FieldDecl& node) override {};
    void visit(StructDecl& node) override {};
    void visit(EnumValueDecl& node) override {};
    void visit(EnumDecl& node) override {};

    void visit(BlockStmt& node) override;
    void visit(BreakStmt& node) override;
    void visit(ContinueStmt& node) override;
    void visit(DeclStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(RuneStmt& node) override;

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

/// A code generation pass over a syntax tree root to fill out the complex of
/// a SIIR control flow graph.
class Codegen final : public Visitor {
    enum ValueContext : u8 { LValue, RValue } m_vctx;
    enum Phase : u8 { PH_Declare, PH_Define } m_phase; 
    Options& m_opts;
    Root& m_root;
    siir::CFG& m_cfg;
    siir::InstBuilder m_builder;
    siir::Function* m_func = nullptr;
    siir::Value* m_tmp = nullptr;
    siir::Value* m_place = nullptr;
    siir::BasicBlock* m_cond = nullptr;
    siir::BasicBlock* m_merge = nullptr;
    
    /// Stores cached results of the `mangle` function.
    std::unordered_map<const Decl*, std::string> m_mangled;

    /// Mangle the name of \p decl as per its runes, and cache it for later
    /// references.
    const std::string& mangle(const Decl* decl);

    /// Lower \p type to its SIIR equivelant, if it can be.
    const siir::Type* lower_type(const Type* type);

    /// Inject a boolean comparison onto \p value if it isn't already typed
    /// with `i1`. Used for implicit language comparisons, like `if x { ... }`.
    siir::Value* inject_bool_cmp(siir::Value* value);

    /// Lower the function \p decl to a new, empty SIIR function with
    /// arguments. 
    void lower_function(const FunctionDecl& decl);
    
    /// Implement the body of function \p decl.
    void impl_function(const FunctionDecl& decl);
    
    /// Lower the structure \p decl to a new SIIR StructType. 
    void lower_structure(const StructDecl& decl);

    /// Binary operation code generation.
    void codegen_binary_assign(const BinaryExpr& node);
    void codegen_binary_add(const BinaryExpr& node);
    void codegen_binary_add_assign(const BinaryExpr& node);
    void codegen_binary_sub(const BinaryExpr& node);
    void codegen_binary_sub_assign(const BinaryExpr& node);
    void codegen_binary_mul(const BinaryExpr& node);
    void codegen_binary_mul_assign(const BinaryExpr& node);
    void codegen_binary_div(const BinaryExpr& node);
    void codegen_binary_div_assign(const BinaryExpr& node);
    void codegen_binary_mod(const BinaryExpr& node);
    void codegen_binary_mod_assign(const BinaryExpr& node);
    void codegen_binary_eq(const BinaryExpr& node);
    void codegen_binary_ne(const BinaryExpr& node);
    void codegen_binary_lt(const BinaryExpr& node);
    void codegen_binary_lte(const BinaryExpr& node);
    void codegen_binary_gt(const BinaryExpr& node);
    void codegen_binary_gte(const BinaryExpr& node);
    void codegen_binary_bitwise_and(const BinaryExpr& node);
    void codegen_binary_bitwise_and_assign(const BinaryExpr& node);
    void codegen_binary_bitwise_or(const BinaryExpr& node);
    void codegen_binary_bitwise_or_assign(const BinaryExpr& node);
    void codegen_binary_bitwise_xor(const BinaryExpr& node);
    void codegen_binary_bitwise_xor_assign(const BinaryExpr& node);
    void codegen_binary_logical_and(const BinaryExpr& node);
    void codegen_binary_logical_or(const BinaryExpr& node);
    void codegen_binary_left_shift(const BinaryExpr& node);
    void codegen_binary_left_shift_assign(const BinaryExpr& node);
    void codegen_binary_right_shift(const BinaryExpr& node);
    void codegen_binary_right_shift_assign(const BinaryExpr& node);

    /// Unary operation code generation.
    void codegen_unary_increment(const UnaryExpr& node);
    void codegen_unary_decrement(const UnaryExpr& node);
    void codegen_unary_dereference(const UnaryExpr& node);
    void codegen_unary_address_of(const UnaryExpr& node);
    void codegen_unary_negate(const UnaryExpr& node);
    void codegen_unary_logical_not(const UnaryExpr& node);
    void codegen_unary_bitwise_not(const UnaryExpr& node);
public:
    Codegen(Options& opts, Root& root, siir::CFG& cfg);

    void visit(Root& node) override;

    void visit(FunctionDecl& node) override;
    void visit(ParameterDecl& node) override {};
    void visit(VariableDecl& node) override;
    void visit(FieldDecl& node) override {};
    void visit(StructDecl& node) override;
    void visit(EnumValueDecl& node) override {};
    void visit(EnumDecl& node) override {};

    void visit(BlockStmt& node) override;
    void visit(BreakStmt& node) override;
    void visit(ContinueStmt& node) override;
    void visit(DeclStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(RuneStmt& node) override;

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
