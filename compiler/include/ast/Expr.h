#ifndef EXPR_STATIMC_H
#define EXPR_STATIMC_H

/// Expression AST nodes.
/// Copyright 2024 Nick Marino (github.com/nwmarino)

#include "Stmt.h"
#include "../core/Type.h"
#include "../sema/ASTVisitor.h"
#include <memory>
#include <utility>

/// UnaryOp - Enumeration of recognized unary operators.
typedef enum {
  /// `!`
  Bang,

  /// `#`
  Rune,

  /// `@`
  Ref,

  /// `.`
  Access,

  UnknownUnaryOp,
} UnaryOp;


/// BinaryOp - Enumeration of recognized binary operators.
typedef enum {
  /// `=`
  Assign,

  /// `+=`
  AddAssign,

  /// `-=`
  SubAssign,

  /// `*=`
  SlashAssign,

  /// `/=`
  StarAssign,

  /// `==`
  IsEq,

  /// `!=`
  IsNotEq,

  /// `&&`
  LogicAnd,

  /// `||`
  LogicOr,

  /// `<`
  Lt,

  /// `<=`
  LtEquals,

  /// `>`
  Gt,

  /// `>=`
  GtEquals,

  /// `+`
  Plus,

  /// `-`
  Minus,

  /// `*`
  Mult,

  /// `/`
  Div,

  UnknownBinaryOp,
} BinaryOp;


/// Returns true if the given binary operator is a (re)assignment operator.
static bool is_assignment_op(BinaryOp op) {
  return op == BinaryOp::Assign || op == BinaryOp::AddAssign || op == BinaryOp::SubAssign || \
    op == BinaryOp::StarAssign || op == BinaryOp::SlashAssign;
}


/// Base class for expressions; statements that may have a value and type.
class Expr : public Stmt
{
public:
  virtual ~Expr() = default;
  virtual void pass(ASTVisitor *visitor) = 0;
  virtual const Type* get_type() const = 0;
  virtual const std::string to_string() = 0;
  virtual const Metadata get_meta() const = 0;
};


/// NullExpr - Represents a null expression.
///
/// @example `null`
class NullExpr final : public Expr
{
private:
  const Type *T;
  const Metadata meta;

public:
  NullExpr(const Type *T, const Metadata &meta) : T(T), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline const Type* get_type() const override { return T; }
  inline void set_type(const Type *T) { this->T = T; }
  const Metadata get_meta() const override { return meta; }

  /// Returns a string representation of this null expression.
  const std::string to_string() override;
};


/// DefaultExpr - Represents a default expression in pattern matching.
///
/// @example `_`
class DefaultExpr final : public Expr
{
private:
  const Type *T;
  const Metadata meta;

public:
  DefaultExpr(const Type *T, const Metadata &meta) : T(T), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline const Type* get_type() const override { return T; }
  const Metadata get_meta() const override { return meta; }

  /// Returns a string representation of this expression.
  const std::string to_string() override;
};


/// BooleanLiteral - Represents a boolean literal expression.
///
/// @example `true`, `false`
class BooleanLiteral final : public Expr
{
private:
  const unsigned int value;
  const Type *T;
  const Metadata meta;

public:
  BooleanLiteral(bool value, const Type *T, const Metadata &meta)
    : value(value ? 1 : 0), T(T), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline const Type* get_type() const override { return T; }
  const Metadata get_meta() const override { return meta; }

  /// Gets the value of this boolean expression.
  inline bool get_value() const { return value; }

  /// Returns a string representation of this boolean expression.
  const std::string to_string() override;
};


/// IntegerLiteral - Represents an integer literal expression.
///
/// @example `0`, `512`, `1024`
class IntegerLiteral final : public Expr
{
private:
  const long value;
  const bool signedness;
  const Type *T;
  const Metadata meta;

public:
  IntegerLiteral(int value, const Type *T, const Metadata &meta) 
    : value(value), signedness(value < 0), T(T), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline const Type* get_type() const override { return T; }
  const Metadata get_meta() const override { return meta; }

  /// Gets the value of this integer expression.
  inline int get_value() const { return value; }

  /// Gets the signedness of this integer expression.
  inline bool is_signed() const { return signedness; }

  /// Returns a string representation of this integer expression.
  const std::string to_string() override;
};


/// FPLiteral - Represents a floating point literal expression.
///
/// @example `0.0`, `3.14`, `6.28`
class FPLiteral final : public Expr
{
private:
  const double value;
  const Type *T;
  const Metadata meta;

public:
  FPLiteral(double value, const Type *T, const Metadata &meta) \
    : value(value), T(T), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline const Type* get_type() const override { return T; }
  const Metadata get_meta() const override { return meta; }

  /// Gets the value of this floating point expression.
  inline double get_value() const { return value; }

  /// Returns a string representation of this floating point expression.
  const std::string to_string() override;
};


/// CharLiteral - Represents a character literal expression.
///
/// @example `'a'`, `'b'`, `'c'`
class CharLiteral final : public Expr
{
private:
  const char value;
  const Type *T;
  const Metadata meta;

public:
  CharLiteral(char value, const Type *T, const Metadata &meta) 
    : value(value), T(T), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline const Type* get_type() const override { return T; }
  const Metadata get_meta() const override { return meta; }

  /// Gets the value of this character expression.
  inline char get_value() const { return value; }

  /// Returns a string representation of this character expression.
  const std::string to_string() override;
};


/// StringLiteral - Represents a string literal expression.
///
/// @example `"hello, world"`, `"foo"`, `"bar"`
class StringLiteral final : public Expr
{
private:
  const std::string value;
  const Type *T;
  const Metadata meta;

public:
  StringLiteral(const std::string &value, const Type *T, const Metadata &meta) 
    : value(value), T(T), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline const Type* get_type() const override { return T; }
  const Metadata get_meta() const override { return meta; }

  /// Gets the value of this string expression.
  inline const std::string get_value() const { return value; }

  /// Returns a string representation of this string expression.
  const std::string to_string() override;
};


/// DeclRefExpr - Represents a reference to a declaration.
///
/// @example `x`, `y`, `z`
class DeclRefExpr final : public Expr
{
private:
  const std::string ident;
  const Type *T;
  const Metadata meta;
  const bool nested;

public:
  DeclRefExpr(const std::string &ident, const Type *T, const Metadata &meta) 
    : ident(ident), T(T), meta(meta), nested(false){};
    DeclRefExpr(const std::string &ident, const Type *T, const Metadata &meta, const bool is_nested) 
    : ident(ident), T(T), meta(meta), nested(is_nested){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline const Type* get_type() const override { return T; }
  inline void set_type(const Type *T) { this->T = T; }
  const Metadata get_meta() const override { return meta; }


  /// Returns true if this reference expression is nested, and false otherwise.
  ///
  /// This is used for enum variant references.
  inline bool is_nested(void) const { return nested; }

  /// Gets the identifier of this reference expression.
  inline const std::string get_ident() const { return ident; }

  /// Returns true if the expression refers to a `this` instance, and false otherwise.
  inline bool is_this() const { return ident == "this"; }

  /// Returns a string representation of this reference expression.
  const std::string to_string() override;
};


/// BinaryExpr - Represents a binary expression.
///
/// @example `x + y`, `x - y`, `x * y`
class BinaryExpr final : public Expr
{
private:
  const BinaryOp op;
  std::unique_ptr<Expr> lhs;
  std::unique_ptr<Expr> rhs;
  const Type *T;
  const Metadata meta;

public:
  BinaryExpr(const BinaryOp op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs, const Metadata &meta) 
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)), meta(meta), T(this->lhs->get_type()){}
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  const Metadata get_meta() const override { return meta; }

  /// Returns the type of this binary expression. Returns `nullptr` if the operand types mismatch.
  inline const Type* get_type() const override { return T; }

  /// Sets the type of this binary expression.
  inline void set_type(const Type *T) { this->T = T; }

  /// Gets the operator of this binary expression.
  inline const BinaryOp get_op() const { return op; }

  /// Gets the left-hand side of this binary expression.
  inline Expr *get_lhs() { return lhs.get(); }

  /// Gets the right-hand side of this binary expression.
  inline Expr *get_rhs() { return rhs.get(); }

  /// Returns a string representation of this binary expression.
  const std::string to_string() override;
};


/// UnaryExpr - Represents a unary expression.
///
/// @example `!x`, `y++`, `#z`
class UnaryExpr final : public Expr
{
private:
  const UnaryOp op;
  std::unique_ptr<Expr> expr;
  const Type *T;
  const Metadata meta;

public:
  UnaryExpr(const UnaryOp op, std::unique_ptr<Expr> expr, const Metadata &meta) 
    : op(op), expr(std::move(expr)), T(this->expr->get_type()), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline const Type* get_type() const override { return T; }
  inline void set_type(const Type *T) { this->T = T; }
  const Metadata get_meta() const override { return meta; }

  /// Gets the operator of this unary expression.
  inline const UnaryOp get_op() const { return op; }

  /// Returns true if this unary expression is a bang operator.
  inline bool is_bang() const { return op == UnaryOp::Bang; }

  /// Returns true if this unary expression is a rune operator.
  inline bool is_rune() const { return op == UnaryOp::Rune; }

  /// Returns true if this unary expression is a reference operator.
  inline bool is_ref() const { return op == UnaryOp::Ref; }

  /// Gets the expr of this unary expression.
  inline Expr *get_expr() { return expr.get(); }

  /// Returns a string representation of this unary expression.
  const std::string to_string() override;
};


/// InitExpr - Represents an initialization expression.
///
/// @example `Foo { x: 1, y: 2 }`, `Bar { z: 3, w: 4 }`
class InitExpr final : public Expr
{
private:
  const std::string ident;
  const Type *T;
  std::vector<std::pair<std::string, std::unique_ptr<Expr>>> fields;
  const Metadata meta;

public:
  InitExpr(const std::string &ident, const Type *T, 
    std::vector<std::pair<std::string, std::unique_ptr<Expr>>> fields, const Metadata &meta)
    : ident(ident), T(T), fields(std::move(fields)), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline const Type* get_type() const override { return T; }
  inline void set_type(const Type *T) { this->T = T; }
  inline std::string get_ident() { return ident; }
  const Metadata get_meta() const override { return meta; }
  const void add_implicit_null(const std::string &field, std::unique_ptr<NullExpr> null_expr) {
    fields.push_back(std::make_pair(field, std::move(null_expr)));
  }
  
  /// Gets the fields of this initialization expression.
  inline std::vector<std::pair<std::string, Expr *>> get_fields() {
    std::vector<std::pair<std::string, Expr *>> fields = {};
    for (const std::pair<std::string, std::unique_ptr<Expr>> &f : this->fields) {
      fields.push_back(std::make_pair(f.first, f.second.get()));
    }
    return fields;
  }

  /// Returns a string representation of this initialization expression.
  const std::string to_string() override;
};


/// CallExpr - Represents a function call expression.
///
/// @example `foo()`, `bar(x, y, 3)`
class CallExpr : public Expr
{
protected:
  const std::string callee;
  std::vector<std::unique_ptr<Expr>> args;
  const Type* T;
  const Metadata meta;

public:
  CallExpr(const std::string &callee, std::vector<std::unique_ptr<Expr>> args, const Metadata &meta)
    : callee(callee), args(std::move(args)), T(nullptr), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline int get_num_args() const { return args.size(); }
  inline const Metadata get_meta() const override { return meta; }

  /// Return the argument at position <n> embedded in this function call.
  inline Expr *get_arg(std::size_t n) {
    for (int i = 0; i < args.size(); i++) {
      if (i == n) {
        return args.at(i).get();
      }
    }
    return nullptr;
  }

  /// Returns the type of this function call expression. Returns `nullptr` if the callee is undefined yet.
  inline const Type* get_type() const override { return T; }

  /// Gets the callee of this function call expression.
  inline const std::string get_callee() const { return callee; }

  /// Sets the type of this function call expression.
  inline void set_type(const Type *T) { this->T = T; }

  /// Returns a string representation of this function call expression.
  const std::string to_string() override;
};


/// MemberExpr - Represents member access expressions.
///
/// @example `foo.bar`, `baz.qux`
class MemberExpr final : public Expr
{
private:
  std::unique_ptr<Expr> base;
  const std::string member;
  const Type *T;
  const Metadata meta;

public:
  MemberExpr(std::unique_ptr<Expr> base, const std::string &member, const Metadata &meta)
    : base(std::move(base)), member(member), T(nullptr), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  const Metadata get_meta() const override { return meta; }

  /// Returns the type of this member access expression. Returns `nullptr` if the member is undefined yet.
  inline const Type* get_type() const override { return T; }

  /// Gets the base of this member access expression.
  inline Expr *get_base() const { return base.get(); }

  /// Sets the type of this member access expression.
  inline void set_type(const Type *T) { this->T = T; }

  /// Gets the member of this member access expression.
  inline const std::string get_member() const { return member; }

  /// Returns a string representation of this member access expression.
  const std::string to_string() override;
};


/// MemberCallExpr - Represents a member call expression.
///
/// @example `foo.bar()`, `baz.qux()`
class MemberCallExpr final : public CallExpr
{
private:
  std::unique_ptr<Expr> base;

public:
  MemberCallExpr(std::unique_ptr<Expr> base, const std::string &callee,
    std::vector<std::unique_ptr<Expr>> args, const Metadata &meta)
    : base(std::move(base)), CallExpr(callee, std::move(args), meta) {};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  const Metadata get_meta() const override { return meta; }
  
  /// Gets the base of this member call expression.
  inline Expr *get_base() { return base.get(); }

  /// Gets the callee of this member call expression.
  inline const std::string get_callee() const { return callee; }

  /// Returns a string representation of this member call expression.
  const std::string to_string() override;
};


/// ThisExpr - Represents a reference to the current instance.
///
/// @example `this`
class ThisExpr final : public Expr
{
private:
  const Type *T;
  const Metadata meta;

public:
  ThisExpr(const Type *T, const Metadata &meta) : T(T), meta(meta){};
  void pass(ASTVisitor *visitor) override { visitor->visit(this); }
  inline const Type* get_type() const override { return T; }
  inline void set_type(const Type *T) { this->T = T; }
  const Metadata get_meta() const override { return meta; }

  /// Returns a string representation of this reference expression.
  const std::string to_string() override;
};

#endif  // EXPR_STATIMC_H
