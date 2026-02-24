//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_PARSER_H_
#define LACE_PARSER_H_

//
//  This header file declares the Parser class, which is used in tandem with 
//  the lexer to turn source code into a syntax tree.
//

#include "lace/lexer/TokenStream.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/types/SourceLocation.h"

namespace lace {

/// Definition of a parser for a lace translation unit into a syntax tree.
class Parser final {
    TokenStream& m_stream;
    std::string m_file;
    AST* m_ast = nullptr;
    Scope* m_scope = nullptr;
    bool m_allow_inits = true;

public:
    /// Create a new parser instance to work on |source|. 
    ///
    /// Optionally, a |path| may be provided for better diagnostics i.e. 
    /// reading in invalid code from the file which contains |source|.
    Parser(TokenStream& stream, const std::string& file = "");

    /// Attempt to parse and a new abstract syntax tree from the source
    /// this parser was constructed with.
    [[nodiscard]] AST* parse();

private:
    /// Returns the current token in use.
    inline const Token& curr() const {
        return m_stream.get();
    }

    /// Lex the next token.
    inline const Token& next() {
        m_stream.advance();
        return curr(); 
    }

    /// Returns the current location in source, based on the current token.
    inline SourceLocation loc() const { return curr().loc; }

    /// Returns a source span beginning at |pos| and ending at the current
    /// location.
    inline SourceSpan since(SourceLocation pos) const { 
        return SourceSpan(pos, curr().loc); 
    }

    /// Test if the kind of the current token matches with |kind|.
    inline bool match(Token::Kind kind) const { return curr().kind == kind; }

    /// Test if the current token is an identifier and has a value that matches
    /// with |kw|.
    inline bool match(const char* kw) const {
        return curr().kind == Token::Identifier && curr().value == kw;
    }

    /// Expect the kind of the current token to match with |kind|. 
    ///
    /// If the token is a match, it will be consumed and the function will
    /// return true. Otherwise, the routine returns false.
    inline bool expect(Token::Kind kind) {
        if (!match(kind))
            return false;

        next();
        return true;
    }

    /// Expect the current token to be an identifier whose values matches with 
    /// |kw|.
    ///
    /// If the token is a match, it will be consumed and the function will
    /// return true. Otherwise, the routine returns false.
    inline bool expect(const char* kw) {
        if (!match(kw))
            return false;

        next();
        return true;
    }

    /// Test if |ident| is a reserved identifier, i.e. conflicts with a keyword
    /// in the language.
    bool is_reserved(const std::string& ident) const;

    /// Enter a new scope, with the current scope as the parent node. Returns
    /// an unmanaged pointer to the new scope.
    Scope* enter_scope() {
        Scope* scope = new Scope(m_scope);
        assert(scope && "failed to create new scope!");

        if (m_scope)
            m_scope->children().push_back(scope);
        
        m_scope = scope;
        return m_scope;
    }

    /// Exit the current scope, and move up to the parent node.
    ///
    /// If there is no parent scope, then the current scope just becomes null.
    inline void exit_scope() { m_scope = m_scope->parent(); }

    /// Returns the equivelant unary operator for the given token |kind|.
    UnaryOp::Operator get_unary_op(Token::Kind kind) const;

    /// Returns the equivelant binary operator for the given token |kind|.
    BinaryOp::Operator get_binary_op(Token::Kind kind) const;

    /// Returns the integer precedence for the binary operator |op|.
    int8_t get_op_precedence(BinaryOp::Operator op) const;

    /// Parse a set of rune decorators and append them to |runes|. 
    void parse_rune_decorators(std::vector<Rune*>& runes);

    Type* parse_type_specifier();

    Defn* parse_initial_definition();
    
    Defn* parse_function_definition(std::vector<Rune*> runes, uint64_t start);

    Defn* parse_binding_definition(std::vector<Rune*> runes, const Token name);
    Defn* parse_load_definition();
    
    Stmt* parse_initial_statement();
    Stmt* parse_block_statement();
    Stmt* parse_control_statement();
    Stmt* parse_declarative_statement();
    Stmt* parse_rune_statement();

    Expr* parse_initial_expression();
    Expr* parse_primary_expression();
    Expr* parse_identifier_expression();
    Expr* parse_prefix_operator();
    Expr* parse_postfix_operator();
    Expr* parse_binary_operator(Expr* base, int8_t precedence);

    Expr* parse_literal_bool();
    Expr* parse_literal_int();
    Expr* parse_literal_float();
    Expr* parse_literal_char();
    Expr* parse_literal_null();
    Expr* parse_literal_string();

    Expr* parse_type_cast();
    Expr* parse_parentheses();
    Expr* parse_sizeof_operator();
    Expr* parse_named_reference();
    Expr* parse_struct_initializer(uint64_t start);
};

} // namespace lace

#endif // LACE_PARSER_H_
