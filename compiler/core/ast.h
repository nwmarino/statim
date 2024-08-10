/// Copyright 2024 Nick Marino (github.com/nwmarino)

#ifndef STATIMC_AST_H
#define STATIMC_AST_H

#include <string>
#include <vector>
#include <memory>
#include <utility>

/// An abstract syntax tree node.
class AST {
  public:
    virtual ~AST() = default;
    const virtual std::string to_str(int n) = 0;
};

/// A statement.
class Statement : public AST {
  public:
    virtual ~Statement() = default;
    const virtual std::string to_str(int n) = 0;
};

/// An expression (or statement ending with semi).
class Expr : public Statement {
  public:
    virtual ~Expr() = default;
    const virtual std::string to_str(int n) = 0;
};

/// Compound statements.
class CompoundStatement : public Statement {
  std::vector<std::unique_ptr<Statement>> stmts;

  public:
    CompoundStatement(std::vector<std::unique_ptr<Statement>> stmts) : stmts(std::move(stmts)) {};
    const std::string to_str(int n);
};

/// Return statements.
class ReturnStatement : public Statement {
  std::unique_ptr<Expr> expr;

  public:
    ReturnStatement(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {};
    const std::string to_str(int n);
};

/// Variable declaration.
///
/// `fix x: int = 0`, `let y: int = 1`
class AssignmentStatement : public Statement {
  const std::string ident;
  const std::string ty;
  std::unique_ptr<Expr> expr;

  public:
    /// Constructor for reassignment.
    AssignmentStatement(const std::string &ident, std::unique_ptr<Expr> expr)
      : ident(ident), ty(""), expr(std::move(expr)) {};
    /// Constructor for initial assignment.
    AssignmentStatement(const std::string &ident, const std::string &ty, std::unique_ptr<Expr> expr)
      : ident(ident), ty(ty), expr(std::move(expr)) {};
    const std::string to_str(int n);
};

/// Variable expression.
///
/// `x`, `y`, `z`
class VariableExpr : public Expr {
  const std::string ident;

  public:
    VariableExpr(const std::string &ident) : ident(ident) {};
    const std::string to_str(int n);
};

/// Null literal expressions.
///
/// `null`
class NullExpr : public Expr {
  public:
    NullExpr() {};
    const std::string to_str(int n);
};

/// Boolean literal expressions.
///
/// `true`, `false`
class BoolExpr : public Expr {
  bool value;

  public:
    BoolExpr(bool value) : value(value) {};
    const std::string to_str(int n);
};

/// Integer literal expressions.
///
/// @example `0`, `512`, `1024`
class IntegerExpr : public Expr {
  const int value;

  public:
    IntegerExpr(int value) : value(value) {};
    const std::string to_str(int n);
};

/// Floating point literal expressions.
///
/// @example `0.0`, `3.14`, `6.28`
class FloatingPointExpr : public Expr {
  const double value;

  public:
    FloatingPointExpr(double value) : value(value) {};
    const std::string to_str(int n);
};

/// Character literal expressions.
///
/// @example `'a'`, `'b'`, `'c'`
class CharExpr : public Expr {
  const char value;

  public:
    CharExpr(char value) : value(value) {};
    const std::string to_str(int n);
};

/// String literal expressions.
///
/// @example `"hello, world"`, `"foo"`, `"bar"`
class StringExpr : public Expr {
  const std::string value;

  public:
    StringExpr(const std::string &value) : value(value) {};
    const std::string to_str(int n);
};

/// Byte literal expressions.
///
/// @example `b'a'`, `b'b'`, `b'c'`
class ByteExpr : public Expr {
  const char value;

  public:
    ByteExpr(char value) : value(value) {};
    const std::string to_str(int n);
};

/// Byte string literal expressions.
///
/// @example `b"hello, world"`, `b"foo"`, `b"bar"`
class ByteStringExpr : public Expr {
  const std::string value;

  public:
    ByteStringExpr(const std::string &value) : value(value) {};
    const std::string to_str(int n);
};

/// Binary operation expressions.
///
/// @example `x + y`, `1 - y`, `x * 2`
class BinaryExpr : public Expr {
  char op;
  std::unique_ptr<Expr> left_child, right_child;

  public:
    BinaryExpr(char op, std::unique_ptr<Expr> left_child, std::unique_ptr<Expr> right_child)
      : op(op), left_child(std::move(left_child)), right_child(std::move(right_child)) {};
    const std::string to_str(int n);
};

/// Function call expressions;
///
/// @example `foo()`, `bar(x, y, 3)`
class FunctionCallExpr : public Expr {
  std::string callee;
  std::vector<std::unique_ptr<Expr>> args;

  public:
    FunctionCallExpr(const std::string &callee, std::vector<std::unique_ptr<Expr>> args)
      : callee(callee), args(std::move(args)) {};
    const std::string to_str(int n);
};

/// Functions prototypes.
class PrototypeAST : public AST {
  std::string name;
  std::vector<std::pair<std::string, std::string>> args;

  public:
    PrototypeAST(const std::string &name, std::vector<std::pair<std::string, std::string>> args)
      : name(name), args(std::move(args)) {};
    const std::string to_str(int n);
};

/// Function definitions.
class FunctionAST : public AST {
  std::unique_ptr<PrototypeAST> head;
  std::unique_ptr<Statement> body;

  public:
    FunctionAST(std::unique_ptr<PrototypeAST> head, std::unique_ptr<Statement> body)
      : head(std::move(head)), body(std::move(body)) {};
    const std::string to_str(int n);
};

/// A program (list of definitions).
class ProgAST : public AST {
  std::vector<std::unique_ptr<FunctionAST>> defs;

  public:
    ProgAST(std::vector<std::unique_ptr<FunctionAST>> defs) : defs(std::move(defs)) {};
    const std::string to_str(int n);
};

#endif  // STATIMC_AST_H
