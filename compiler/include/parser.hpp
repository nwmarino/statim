#ifndef STATIM_PARSER_HPP_
#define STATIM_PARSER_HPP_

#include "ast.hpp"
#include "input_file.hpp"
#include "lexer.hpp"
#include "source_loc.hpp"
#include "translation_unit.hpp"

#include <memory>

namespace stm {

class Parser final {
    InputFile&              file;
    Lexer                   lexer;
    std::unique_ptr<Root>   root;
    std::vector<Rune*>      runes;
    Scope*                  pScope = nullptr;

public:
    Parser(InputFile& file);
    
    void parse(TranslationUnit& unit);

private:
    bool match(TokenKind kind) const;
    
    bool match(const char* kw) const;

    void next();

    void skip(u32 n);

    Span since(const SourceLocation& loc);

    Scope* enter_scope(Scope::Context context);

    void exit_scope();

    BinaryExpr::Operator binop(TokenKind kind) const;

    i32 binop_precedence(TokenKind kind) const;

    UnaryExpr::Operator unop(TokenKind kind) const;

    void parse_rune_decorators();

    const Type* parse_type();

    Decl*           parse_decl();
    FunctionDecl*   parse_function(const Token& name);
    ParameterDecl*  parse_parameter();
    VariableDecl*   parse_variable();

    Stmt*           parse_stmt();
    BlockStmt*      parse_block();
    BreakStmt*      parse_break();
    ContinueStmt*   parse_continue();
    DeclStmt*       parse_decl_stmt();
    IfStmt*         parse_if();
    WhileStmt*      parse_while();
    RetStmt*        parse_ret();

    Expr* parse_expr();
    Expr* parse_primary();
    Expr* parse_identifier();
    Expr* parse_binary(Expr* pBase, i32 precedence);
    Expr* parse_unary_prefix();
    Expr* parse_unary_postfix();

    BoolLiteral* parse_bool();
    IntegerLiteral* parse_integer();
    FloatLiteral* parse_float();
    CharLiteral* parse_char();
    StringLiteral* parse_string();
    NullLiteral* parse_null();

    CastExpr* parse_cast();
    ParenExpr* parse_paren();
    SizeofExpr* parse_sizeof();
    ReferenceExpr* parse_ref();
    CallExpr* parse_call();
};

} // namespace stm

#endif // STATIM_PARSER_HPP_
