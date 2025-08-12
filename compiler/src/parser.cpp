#include "parser.hpp"
#include "ast.hpp"
#include "logger.hpp"
#include "source_loc.hpp"
#include "token.hpp"
#include "type.hpp"

#include <string>

using namespace stm;

Parser::Parser(InputFile& file) : lexer(file), runes() {
    root = std::make_unique<Root>(file, enter_scope(Scope::Context::Global));
    lexer.lex();

    while (!lexer.is_eof()) {
        Decl* decl = parse_decl();
        if (!decl)
            logger_fatal("expected declaration", &lexer.last().loc);

        root->add_decl(decl);
    }
}

bool Parser::match(TokenKind kind) const {
    return lexer.last().kind == kind;
}
    
bool Parser::match(const char* kw) const {
    const Token& tk = lexer.last();
    return (tk.kind == TOKEN_KIND_IDENTIFIER && tk.value == kw);
}

void Parser::expect(TokenKind kind) const {
    if (lexer.last().kind != kind)
        logger_fatal("expected '" + token_kind_to_string(kind) + "'", &lexer.last().loc);
}

void Parser::next() {
    lexer.lex();
}

void Parser::skip(u32 n) {
    for (u32 idx = 0; idx != n; ++idx)
        lexer.lex();
}

Scope* Parser::enter_scope(Scope::Context context) {
    pScope = new Scope(context, pScope);
    return pScope;
}

void Parser::exit_scope() {
    pScope = pScope->get_parent();
}

BinaryExpr::Operator Parser::binop(TokenKind kind) const {
    switch (kind) {
    case TOKEN_KIND_EQUALS: return BinaryExpr::Operator::Assign;
    case TOKEN_KIND_EQUALS_EQUALS: return BinaryExpr::Operator::Equals;
    case TOKEN_KIND_BANG_EQUALS: return BinaryExpr::Operator::Not_Equals;
    case TOKEN_KIND_PLUS: return BinaryExpr::Operator::Add;
    case TOKEN_KIND_PLUS_EQUALS: return BinaryExpr::Operator::Add_Assign;
    case TOKEN_KIND_MINUS: return BinaryExpr::Operator::Sub;
    case TOKEN_KIND_MINUS_EQUALS: return BinaryExpr::Operator::Sub_Assign;
    case TOKEN_KIND_STAR: return BinaryExpr::Operator::Mul;
    case TOKEN_KIND_STAR_EQUALS: return BinaryExpr::Operator::Mul_Assign;
    case TOKEN_KIND_SLASH: return BinaryExpr::Operator::Div;
    case TOKEN_KIND_SLASH_EQUALS: return BinaryExpr::Operator::Div_Assign;
    case TOKEN_KIND_PERCENT: return BinaryExpr::Operator::Mod;
    case TOKEN_KIND_PERCENT_EQUALS: return BinaryExpr::Operator::Mod_Assign;
    case TOKEN_KIND_AND: return BinaryExpr::Operator::Bitwise_And;
    case TOKEN_KIND_AND_AND: return BinaryExpr::Operator::Logical_And;
    case TOKEN_KIND_AND_EQUALS: return BinaryExpr::Operator::Bitwise_And_Assign;
    case TOKEN_KIND_OR: return BinaryExpr::Operator::Bitwise_Or;
    case TOKEN_KIND_OR_OR: return BinaryExpr::Operator::Logical_Or;
    case TOKEN_KIND_OR_EQUALS: return BinaryExpr::Operator::Bitwise_Or_Assign;
    case TOKEN_KIND_XOR: return BinaryExpr::Operator::Bitwise_Xor;
    case TOKEN_KIND_XOR_EQUALS: return BinaryExpr::Operator::Bitwise_Xor_Assign;
    case TOKEN_KIND_LEFT: return BinaryExpr::Operator::Less_Than;
    case TOKEN_KIND_LEFT_LEFT: return BinaryExpr::Operator::Left_Shift;
    case TOKEN_KIND_LEFT_LEFT_EQUALS: return BinaryExpr::Operator::Left_Shift_Assign;
    case TOKEN_KIND_LEFT_EQUALS: return BinaryExpr::Operator::Less_Than_Equals;
    case TOKEN_KIND_RIGHT: return BinaryExpr::Operator::Greater_Than;
    case TOKEN_KIND_RIGHT_RIGHT: return BinaryExpr::Operator::Right_Shift;
    case TOKEN_KIND_RIGHT_RIGHT_EQUALS: return BinaryExpr::Operator::Right_Shift_Assign;
    case TOKEN_KIND_RIGHT_EQUALS: return BinaryExpr::Operator::Greater_Than;
    default: return BinaryExpr::Operator::Unknown;
    }
}

i32 Parser::binop_precedence(TokenKind kind) const {
    switch (kind) {
    case TOKEN_KIND_STAR:
    case TOKEN_KIND_SLASH:
    case TOKEN_KIND_PERCENT:
        return 11;

    case TOKEN_KIND_PLUS:
    case TOKEN_KIND_MINUS:
        return 10;

    case TOKEN_KIND_LEFT_LEFT:
    case TOKEN_KIND_RIGHT_RIGHT:
        return 9;

    case TOKEN_KIND_LEFT:
    case TOKEN_KIND_LEFT_EQUALS:
    case TOKEN_KIND_RIGHT:
    case TOKEN_KIND_RIGHT_EQUALS:
        return 8;

    case TOKEN_KIND_EQUALS_EQUALS:
    case TOKEN_KIND_BANG_EQUALS:
        return 7;
    
    case TOKEN_KIND_AND:
        return 6;
    
    case TOKEN_KIND_XOR:
        return 5;
    
    case TOKEN_KIND_OR:
        return 4;
        
    case TOKEN_KIND_AND_AND:
        return 3;

    case TOKEN_KIND_OR_OR:
        return 2;
        
    case TOKEN_KIND_EQUALS:
    case TOKEN_KIND_PLUS_EQUALS:
    case TOKEN_KIND_MINUS_EQUALS:
    case TOKEN_KIND_STAR_EQUALS:
    case TOKEN_KIND_SLASH_EQUALS:
    case TOKEN_KIND_PERCENT_EQUALS:
    case TOKEN_KIND_AND_EQUALS:
    case TOKEN_KIND_OR_EQUALS:
    case TOKEN_KIND_XOR_EQUALS:
    case TOKEN_KIND_LEFT_LEFT_EQUALS:
    case TOKEN_KIND_RIGHT_RIGHT_EQUALS:
        return 1;

    default: 
        return -1;
    }
}

UnaryExpr::Operator Parser::unop(TokenKind kind) const {
    switch (kind) {
    case TOKEN_KIND_BANG: return UnaryExpr::Operator::Logical_Not;
    case TOKEN_KIND_PLUS_PLUS: return UnaryExpr::Operator::Increment;
    case TOKEN_KIND_MINUS_MINUS: return UnaryExpr::Operator::Decrement;
    case TOKEN_KIND_STAR: return UnaryExpr::Operator::Dereference;
    case TOKEN_KIND_AND: return UnaryExpr::Operator::Address_Of;
    case TOKEN_KIND_TILDE: return UnaryExpr::Operator::Bitwise_Not;
    default: return UnaryExpr::Operator::Unknown;
    }
}

void Parser::parse_rune_decorators() {

}

const Type* Parser::parse_type() {
    DeferredType::Context context {
        .meta = lexer.last().loc,
        .mut = match("mut"),
        .pScope = pScope,
        .indirection = 0,
    };

    if (context.mut)
        next(); // 'mut'

    while (match(TOKEN_KIND_STAR)) {
        context.indirection++;
        next(); // '*'
    }

    expect(TOKEN_KIND_IDENTIFIER);
    context.base = lexer.last().value;
    next(); // identifier

    return DeferredType::get(*root, context);
}

Decl* Parser::parse_decl() {
    if (match(TOKEN_KIND_SIGN))
        parse_rune_decorators();

    expect(TOKEN_KIND_IDENTIFIER);
   
    const Token name = lexer.last();
    next();

    expect(TOKEN_KIND_PATH);
    next();

    switch (lexer.last().kind) {
    case TOKEN_KIND_SET_PAREN:
        return parse_function(name);
    default:
        return nullptr;
    }
}

FunctionDecl* Parser::parse_function(const Token& name) {
    next(); // '('

    std::vector<Rune*> function_runes = runes;
    runes.clear();
    Scope* scope = enter_scope(Scope::Context::Function);
    Stmt* body = nullptr;
    std::vector<ParameterDecl*> params {};

    while (!match(TOKEN_KIND_END_PAREN)) {
        ParameterDecl* param = parse_parameter();
        if (!param)
            logger_fatal("expected parameter");

        params.push_back(param);

        expect(TOKEN_KIND_COMMA);
        next(); // ','
    }

    next(); // ')'

    expect(TOKEN_KIND_ARROW);
    next(); // '->'
    const Type* return_type = parse_type();

    std::vector<const Type*> param_types { params.size() };
    for (auto& param : params)
        param_types.push_back(param->get_type());

    const FunctionType* type = FunctionType::get(
        *root, return_type, param_types);
    
    expect(TOKEN_KIND_SET_BRACE);
    body = parse_stmt();
    if (!body)
        logger_fatal("expected function body");

    exit_scope();
    FunctionDecl* function = new FunctionDecl(
        Span(name.loc, body->get_span().end),
        name.value,
        runes,
        type,
        params,
        scope,
        body
    );

    pScope->add(function);
    return function;
}

/// Parse a parameter that appears like:
///
/// <name> ':' <...type>
ParameterDecl* Parser::parse_parameter() {
    expect(TOKEN_KIND_IDENTIFIER);
    const Token& name = lexer.last();
    next(); // identifier

    expect(TOKEN_KIND_COLON);
    next(); // ':'

    const Type* type = parse_type();
    ParameterDecl* param = new ParameterDecl(
        Span(name.loc, lexer.last(1).loc),
        name.value,
        {},
        type
    );

    pScope->add(param);
    return param;
}

VariableDecl* Parser::parse_variable() {
    return nullptr;
}

Stmt* Parser::parse_stmt() {
    if (match(TOKEN_KIND_SET_BRACE))
        return parse_block();
    else if (match("break"))
        return parse_break();
    else if (match("continue"))
        return parse_continue();
    else if (match("let"))
        return parse_decl_stmt();
    else if (match("if"))
        return parse_if();
    else if (match("while"))
        return parse_while();
    else if (match("ret"))
        return parse_ret();
    else
        return parse_expr();
}

BlockStmt* Parser::parse_block() {
    parse_rune_decorators();

    SourceLocation begin = lexer.last().loc;
    Scope* scope = enter_scope(Scope::Context::Block);
    std::vector<Stmt*> stmts;
    next(); // '{'

    while (!match(TOKEN_KIND_END_BRACE)) {
        Stmt* stmt = parse_stmt();
        if (!stmt)
            logger_fatal("expected statement", &lexer.last().loc);

        while (match(TOKEN_KIND_SEMICOLON))
            next(); // ';'

        stmts.push_back(stmt);
    }
    
    SourceLocation end = lexer.last().loc;
    next(); // '}'
    exit_scope();
    return new BlockStmt(
        Span(begin, end),
        {},
        stmts,
        scope);
}

BreakStmt* Parser::parse_break() {
    return nullptr;
}

ContinueStmt* Parser::parse_continue() {
    return nullptr;
}

DeclStmt* Parser::parse_decl_stmt() {
    return nullptr;
}

IfStmt* Parser::parse_if() {
    return nullptr;
}

WhileStmt* Parser::parse_while() {
    return nullptr;
}

RetStmt* Parser::parse_ret() {
    SourceLocation begin = lexer.last().loc;
    next(); // 'ret'

    Expr* expr = nullptr;
    if (!match(TOKEN_KIND_SEMICOLON)) {
        expr = parse_expr();
        if (!expr) {
            logger_error("expected expression after 'ret'", &lexer.last().loc);
            return nullptr;
        }
    }

    expect(TOKEN_KIND_SEMICOLON);
    SourceLocation end = lexer.last().loc;
    next(); // ';'
    return new RetStmt(Span(begin, end), expr);
}

Expr* Parser::parse_expr() {
    Expr* base = parse_unary_prefix();
    if (!base) {
        logger_error("expected expression", &lexer.last().loc);
        return nullptr;
    }

    return parse_binary(base, 0);
}

Expr* Parser::parse_primary() {
    if (match(TOKEN_KIND_IDENTIFIER))
        return parse_identifier();
    else if (match(TOKEN_KIND_SET_PAREN))
        return parse_paren();
    else if (match(TOKEN_KIND_INTEGER))
        return parse_integer();
    else if (match(TOKEN_KIND_FLOAT))
        return parse_float();
    else if (match(TOKEN_KIND_CHARACTER))
        return parse_char();
    else if (match(TOKEN_KIND_STRING))
        return parse_string();
    else
        return nullptr;
}

Expr* Parser::parse_identifier() {
    if (match("cast"))
        return parse_cast();
    else if (match("nil"))
        return parse_nil();
    else if (match("true") || match("false"))
        return parse_bool();
    else if (match("sizeof"))
        return parse_sizeof();
    else
        return nullptr;
}

Expr* Parser::parse_binary(Expr* pBase, i32 precedence) {
    while (true) {
        i32 tok_prec = binop_precedence(lexer.last().kind);
        if (tok_prec < precedence)
            break;

        BinaryExpr::Operator op = binop(lexer.last().kind);
        if (op == BinaryExpr::Operator::Unknown)
            break;

        next(); // op
        Expr* right = parse_unary_prefix();
        if (!right)
            logger_fatal("expected binary right side expression", &lexer.last().loc);

        i32 next_prec = binop_precedence(lexer.last().kind);
        if (tok_prec < next_prec) {
            right = parse_binary(right, precedence + 1);
            if (right)
                logger_fatal("expected binary expression", &lexer.last().loc);
        }

        pBase = new BinaryExpr(
            Span(pBase->get_span().begin, right->get_span().end),
            nullptr,
            Expr::ValueKind::RValue,
            op,
            pBase,
            right);
    }

    return pBase;
}

Expr* Parser::parse_unary_prefix() {
    UnaryExpr::Operator op = unop(lexer.last().kind);
    if (UnaryExpr::is_prefix(op)) {
        SourceLocation begin = lexer.last().loc;
        next(); // op

        Expr* base = parse_unary_prefix();
        if (!base)
            logger_fatal("expected unary prefix expression", &lexer.last().loc);

        return new UnaryExpr(
            Span(begin, base->get_span().end),
            nullptr,
            Expr::ValueKind::RValue,
            op,
            base,
            false);
    } else {
        return parse_unary_postfix();
    }
}

Expr* Parser::parse_unary_postfix() {
    Expr* expr = parse_primary();
    if (!expr) {
        logger_error("expected primary expression", &lexer.last().loc);
        return nullptr;
    }

    while (true) {
        SourceLocation begin = lexer.last().loc;
        UnaryExpr::Operator op = unop(lexer.last().kind);
        if (UnaryExpr::is_postfix(op)) {
            next(); // op
            expr = new UnaryExpr(
                Span(begin),
                nullptr,
                Expr::ValueKind::RValue,
                op,
                expr,
                true);
        } else if (match(TOKEN_KIND_SET_BRACKET)) {
            // Token is not an operator, but a subscript '[' ... ']'
        } else if (match(TOKEN_KIND_DOT)) {
            // Token is not an operator, but a member access '.'
        } else {
            break;
        }
    }

    return expr;
}

BoolLiteral* Parser::parse_bool() {
    BoolLiteral* boolean = new BoolLiteral(
        Span(lexer.last().loc),
        root->get_bool_type(),
        match("true")
    );
    next(); // 'true' | 'false'
    return boolean;
}

IntegerLiteral* Parser::parse_integer() {
    IntegerLiteral* integer = new IntegerLiteral(
        Span(lexer.last().loc),
        root->get_si64_type(),
        std::stol(lexer.last().value, 0, 10)
    );
    next(); // integer
    return integer;
}

FloatLiteral* Parser::parse_float() {
    FloatLiteral* fp = new FloatLiteral(
        Span(lexer.last().loc),
        root->get_fp64_type(),
        std::stod(lexer.last().value, 0)
    );
    next(); // float
    return fp;
}

CharLiteral* Parser::parse_char() {
    CharLiteral* character = new CharLiteral(
        Span(lexer.last().loc),
        root->get_char_type(),
        lexer.last().value.at(0)
    );
    next(); // char
    return character;
}

StringLiteral* Parser::parse_string() {
    StringLiteral* string = new StringLiteral(
        Span(lexer.last().loc),
        PointerType::get(*root, root->get_char_type()),
        lexer.last().value
    );
    next(); // string
    return string;
}

NilLiteral* Parser::parse_nil() {
    NilLiteral* nil = new NilLiteral(
        Span(lexer.last().loc),
        PointerType::get(*root, root->get_void_type())
    );
    next(); // 'nil'
    return nil;
}

CastExpr* Parser::parse_cast() {
    return nullptr;
}

ParenExpr* Parser::parse_paren() {
    SourceLocation begin = lexer.last().loc;
    next(); // '('

    Expr* expr = parse_expr();
    if (!expr)
        logger_fatal("expected expression after '('", &begin);

    expect(TOKEN_KIND_END_PAREN);
    SourceLocation end = lexer.last().loc;
    next(); // ')'

    return new ParenExpr(
        Span(begin, end),
        expr
    );
}

SizeofExpr* Parser::parse_sizeof() {
    SourceLocation begin = lexer.last().loc;
    next(); // 'sizeof'

    expect(TOKEN_KIND_SET_PAREN);
    next(); // '('

    const Type* type = parse_type();

    expect(TOKEN_KIND_END_PAREN);
    SourceLocation end = lexer.last().loc;
    next(); // ')'

    return new SizeofExpr(
        Span(begin, end),
        root->get_ui64_type(),
        type
    );
}

ReferenceExpr* Parser::parse_ref() {
    return nullptr;
}

CallExpr* Parser::parse_call() {
    return nullptr;
}
