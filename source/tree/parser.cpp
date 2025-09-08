#include "core/logger.hpp"
#include "tree/type.hpp"
#include "tree/parser.hpp"
#include "types/source_location.hpp"
#include "types/token.hpp"
#include "types/types.hpp"

#include <cassert>
#include <string>

using namespace stm;

Parser::Parser(InputFile& file) : file(file), lexer(file), runes() {
    lexer.lex();
}

void Parser::parse(TranslationUnit& unit) {
    root = std::make_unique<Root>(file, enter_scope());

    while (!lexer.is_eof()) {
        Decl* decl = parse_decl();
        assert(decl && "could not parse declaration");
        root->add_decl(decl);

        if (decl->has_decorator(Rune::Public)) {
            root->exports().push_back(decl);
        }
    }

    unit.set_root(std::move(root));
    root = nullptr;
}

bool Parser::match(TokenKind kind) const {
    return lexer.last().kind == kind;
}
    
bool Parser::match(const char* kw) const {
    const Token& tk = lexer.last();
    return (tk.kind == TOKEN_KIND_IDENTIFIER && tk.value == kw);
}

void Parser::next() {
    lexer.lex();
}

void Parser::skip(u32 n) {
    for (u32 idx = 0; idx != n; ++idx) lexer.lex();
}

Span Parser::since(const SourceLocation& loc) {
    return Span(loc, lexer.last().loc);
}

Scope* Parser::enter_scope() {
    pScope = new Scope(pScope);
    return pScope;
}

void Parser::exit_scope() {
    pScope = pScope->get_parent();
}

BinaryExpr::Operator Parser::binop(TokenKind kind) const {
    switch (kind) {
    case TOKEN_KIND_EQUALS: 
        return BinaryExpr::Operator::Assign;
    case TOKEN_KIND_EQUALS_EQUALS: 
        return BinaryExpr::Operator::Equals;
    case TOKEN_KIND_BANG_EQUALS: 
        return BinaryExpr::Operator::Not_Equals;
    case TOKEN_KIND_PLUS: 
        return BinaryExpr::Operator::Add;
    case TOKEN_KIND_PLUS_EQUALS: 
        return BinaryExpr::Operator::Add_Assign;
    case TOKEN_KIND_MINUS: 
        return BinaryExpr::Operator::Sub;
    case TOKEN_KIND_MINUS_EQUALS: 
        return BinaryExpr::Operator::Sub_Assign;
    case TOKEN_KIND_STAR: 
        return BinaryExpr::Operator::Mul;
    case TOKEN_KIND_STAR_EQUALS: 
        return BinaryExpr::Operator::Mul_Assign;
    case TOKEN_KIND_SLASH: 
        return BinaryExpr::Operator::Div;
    case TOKEN_KIND_SLASH_EQUALS: 
        return BinaryExpr::Operator::Div_Assign;
    case TOKEN_KIND_PERCENT: 
        return BinaryExpr::Operator::Mod;
    case TOKEN_KIND_PERCENT_EQUALS: 
        return BinaryExpr::Operator::Mod_Assign;
    case TOKEN_KIND_AND: 
        return BinaryExpr::Operator::Bitwise_And;
    case TOKEN_KIND_AND_AND: 
        return BinaryExpr::Operator::Logical_And;
    case TOKEN_KIND_AND_EQUALS: 
        return BinaryExpr::Operator::Bitwise_And_Assign;
    case TOKEN_KIND_OR: 
        return BinaryExpr::Operator::Bitwise_Or;
    case TOKEN_KIND_OR_OR: 
        return BinaryExpr::Operator::Logical_Or;
    case TOKEN_KIND_OR_EQUALS: 
        return BinaryExpr::Operator::Bitwise_Or_Assign;
    case TOKEN_KIND_XOR: 
        return BinaryExpr::Operator::Bitwise_Xor;
    case TOKEN_KIND_XOR_EQUALS: 
        return BinaryExpr::Operator::Bitwise_Xor_Assign;
    case TOKEN_KIND_LEFT: 
        return BinaryExpr::Operator::Less_Than;
    case TOKEN_KIND_LEFT_LEFT: 
        return BinaryExpr::Operator::Left_Shift;
    case TOKEN_KIND_LEFT_LEFT_EQUALS: 
        return BinaryExpr::Operator::Left_Shift_Assign;
    case TOKEN_KIND_LEFT_EQUALS: 
        return BinaryExpr::Operator::Less_Than_Equals;
    case TOKEN_KIND_RIGHT: 
        return BinaryExpr::Operator::Greater_Than;
    case TOKEN_KIND_RIGHT_RIGHT: 
        return BinaryExpr::Operator::Right_Shift;
    case TOKEN_KIND_RIGHT_RIGHT_EQUALS: 
        return BinaryExpr::Operator::Right_Shift_Assign;
    case TOKEN_KIND_RIGHT_EQUALS: 
        return BinaryExpr::Operator::Greater_Than;
    default: 
        return BinaryExpr::Operator::Unknown;
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
    case TOKEN_KIND_BANG: 
        return UnaryExpr::Operator::Logical_Not;
    case TOKEN_KIND_PLUS_PLUS: 
        return UnaryExpr::Operator::Increment;
    case TOKEN_KIND_MINUS: 
        return UnaryExpr::Operator::Negate;
    case TOKEN_KIND_MINUS_MINUS: 
        return UnaryExpr::Operator::Decrement;
    case TOKEN_KIND_STAR: 
        return UnaryExpr::Operator::Dereference;
    case TOKEN_KIND_AND: 
        return UnaryExpr::Operator::Address_Of;
    case TOKEN_KIND_TILDE: 
        return UnaryExpr::Operator::Bitwise_Not;
    default: 
        return UnaryExpr::Operator::Unknown;
    }
}

Rune* Parser::parse_rune() {
    if (!match(TOKEN_KIND_IDENTIFIER)) {
        Logger::info(token_kind_to_string(lexer.last().kind));
        Logger::fatal(
                "expected rune identifier after '$'", since(lexer.last().loc));
    }

    Rune::Kind kind = Rune::from_string(lexer.last().value);
    if (kind == Rune::Unknown) {
        Logger::fatal(
            "unrecognized rune: '$" + lexer.last().value + "'",
            since(lexer.last().loc));
    }

    next(); // identifier
    std::vector<Expr*> args = {};
    if (match(TOKEN_KIND_SET_PAREN)) {
        next(); // '('

        while (!match(TOKEN_KIND_END_PAREN)) {
            auto* expr = parse_expr();
            assert(expr && "could not parse rune argument!");
            args.push_back(expr);

            if (match(TOKEN_KIND_END_PAREN))
                break;

            if (!match(TOKEN_KIND_COMMA)) {
                Logger::fatal(
                    "expected ',' or ')' after rune argument list",
                    lexer.last().loc);
            }

            next(); // ','
        }

        next(); // ')'
    };

    if (!Rune::accepts_args(kind) && !args.empty()) {
        Logger::fatal(
            "rune '" + Rune::to_string(kind) + "' does not accept arguments", 
            since(lexer.last().loc)); 
    }

    return new Rune(kind, args);
}

void Parser::parse_rune_decorators() {
    runes.clear();

    if (!match(TOKEN_KIND_SIGN))
        return;

    next(); // '$'

    if (match(TOKEN_KIND_SET_BRACKET)) {
        next(); // '['

        while (!match(TOKEN_KIND_END_BRACKET)) {
            auto* rune = parse_rune();
            if (!Rune::is_decorator(rune->kind())) {
                Logger::fatal(
                    "non-decorator rune in decorator list", 
                    since(lexer.last().loc));
            }

            runes.push_back(rune);

            if (match(TOKEN_KIND_END_BRACKET))
                break;

            if (!match(TOKEN_KIND_COMMA)) {
                Logger::fatal(
                    "expected ',' or ']' after rune decorator list", 
                    since(lexer.last().loc));
            }

            next(); // ','
        }

        next(); // ']'
    } else {
        auto* rune = parse_rune();
        if (!Rune::is_decorator(rune->kind())) {
            Logger::fatal(
                "non-decorator rune in decorator list", 
                since(lexer.last().loc));
        }

        runes.push_back(parse_rune());
    }
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

    if (!match(TOKEN_KIND_IDENTIFIER))
        Logger::fatal("expected type identifier");

    context.base = lexer.last().value;
    next(); // identifier

    return DeferredType::get(*root, context);
}

Decl* Parser::parse_decl() { 
    parse_rune_decorators();

    if (!match(TOKEN_KIND_IDENTIFIER)) {
        Logger::fatal(
            "expected declaration name identifier",
            Span(lexer.last().loc));
    }

    if (lexer.last().value == "use")
        return parse_use();

    const Token name = lexer.last();
    next(); // identifier

    if (!match(TOKEN_KIND_PATH)) {
        Logger::fatal(
            "expected '::' operator after declaration name",
            since(name.loc));
    }

    next(); // '::'

    switch (lexer.last().kind) {
    case TOKEN_KIND_SET_PAREN:
        return parse_function(name);
    case TOKEN_KIND_SET_BRACE:
        return parse_struct(name);
    case TOKEN_KIND_IDENTIFIER:
        return parse_enum(name);
    default:
        Logger::fatal(
            "expected declaration after binding operator '::'",
            since(name.loc));
    }
}

UseDecl* Parser::parse_use() {
    SourceLocation loc = lexer.last().loc;
    next(); // 'use'

    std::vector<Rune*> use_runes = this->runes;
    this->runes.clear();

    if (!match(TOKEN_KIND_STRING)) {
        Logger::fatal(
            "expected string literal enclosed by '\"' after 'use' keyword",
            since(lexer.last().loc));
    }

    std::string path = lexer.last().value;
    next(); // "path"

    if (!match(TOKEN_KIND_SEMICOLON)) {
        Logger::fatal("expected ';' after 'use' declaration", since(loc));
    }

    next(); // ';'

    return new UseDecl(since(loc), path, use_runes);
}

FunctionDecl* Parser::parse_function(const Token& name) {
    next(); // '('

    std::vector<Rune*> function_runes = runes;
    runes.clear();
    Scope* scope = enter_scope();
    Stmt* body = nullptr;
    std::vector<ParameterDecl*> params {};

    while (!match(TOKEN_KIND_END_PAREN)) {
        if (!match(TOKEN_KIND_IDENTIFIER)) {
            Logger::fatal(
                "expected parameter name identifier",
                since(name.loc));
        }

        const Token pname = lexer.last();
        next(); // identifier

        if (!match(TOKEN_KIND_COLON)) {
            Logger::fatal(
                "expected ':' after parameter name", 
                since(pname.loc));
        }

        next(); // ':'

        const Type* type = parse_type();
        ParameterDecl* param = new ParameterDecl(
            Span(pname.loc, lexer.last(1).loc),
            pname.value,
            {},
            type
        );

        if (!pScope->add(param)) {
            Logger::fatal(
                "function parameter reuses existing name in scope: '" + 
                    pname.value + "'",
                since(name.loc));
        }

        params.push_back(param);

        if (match(TOKEN_KIND_END_PAREN)) break;

        if (!match(TOKEN_KIND_COMMA)) {
            Logger::fatal("expected ',' after function parameter", 
                since(param->get_span().begin));
        }

        next(); // ','
    }

    next(); // ')'

    if (!match(TOKEN_KIND_ARROW)) {
        Logger::fatal("expected '->' to define function return type", 
            since(name.loc));
    }

    next(); // '->'
    const Type* return_type = parse_type();

    std::vector<const Type*> param_types;
    param_types.reserve(params.size());
    for (auto& param : params)
        param_types.push_back(param->get_type());

    const FunctionType* type = FunctionType::get(
        *root, return_type, param_types);
    
    if (match(TOKEN_KIND_SET_BRACE)) {
        body = parse_stmt();
        assert(body && "could not parse function body");
    } else if (!match(TOKEN_KIND_SEMICOLON)) {
        Logger::fatal("expected '{' or ';' after function signature", 
            since(name.loc));
    } else {
        next(); // ';'
    }

    exit_scope();
    FunctionDecl* function = new FunctionDecl(
        Span(name.loc, body != nullptr ? body->get_span().end : name.loc),
        name.value,
        function_runes,
        type,
        params,
        scope,
        body
    );

    if (!pScope->add(function)) {
        Logger::fatal(
            "function reuses existing name in scope: '" + name.value + "'",
            since(name.loc));
    }

    return function;
}

VariableDecl* Parser::parse_variable() {
    SourceLocation begin = lexer.last().loc;
    next(); // 'let'

    if (!match(TOKEN_KIND_IDENTIFIER))
        Logger::fatal("expected variable name after 'let'", since(begin));

    std::string name = lexer.last().value;
    const Type* type = nullptr;
    Expr* init = nullptr;

    next(); // name
    
    if (match(TOKEN_KIND_COLON)) {
        // After the variable name there is a ':', which means a type was given.
        next(); // ':'
        type = parse_type();
    }

    if (!match(TOKEN_KIND_EQUALS) && !match(TOKEN_KIND_SEMICOLON))
        Logger::fatal("expected '=' or ';' after variable declaration", since(begin));

    if (match(TOKEN_KIND_EQUALS)) {
        next(); // '='
        init = parse_expr();
        assert(init && "could not parse variable initializer");
    }

    SourceLocation end = lexer.last().loc;

    if (!match(TOKEN_KIND_SEMICOLON))
        Logger::fatal("expected ';' after variable declaration", since(begin));

    VariableDecl* var = new VariableDecl(
        Span(begin, end),
        name,
        {},
        type,
        init);

    pScope->add(var);
    return var;
}

StructDecl* Parser::parse_struct(const Token& name) {
    next(); // '{'

    std::vector<FieldDecl*> fields;

    while (!match(TOKEN_KIND_END_BRACE)) {
        if (!match(TOKEN_KIND_IDENTIFIER)) {
            Logger::fatal(
                "expected field name identifier",
                since(name.loc));
        }

        const Token& fname = lexer.last();
        const Type* ftype = nullptr;
        next(); // identifier

        if (!match(TOKEN_KIND_COLON)) {
            Logger::fatal(
                "expected ':' after field name",
                since(fname.loc));
        }

        next(); // ':'
        ftype = parse_type();

        fields.push_back(new FieldDecl(
            since(fname.loc),
            fname.value,
            {},
            ftype,
            nullptr,
            fields.size()));

        if (match(TOKEN_KIND_END_BRACE)) break;

        if (!match(TOKEN_KIND_COMMA)) {
            Logger::fatal(
                "expected ',' or '}' after structure field",
                since(fname.loc));
        }

        next(); // ','
    }

    SourceLocation end = lexer.last().loc;
    next(); // '}'

    StructDecl* decl = new StructDecl(
        Span(name.loc, end),
        name.value,
        {},
        nullptr,
        fields);

    std::vector<const Type*> field_types { fields.size() };
    for (auto field : fields) field_types.push_back(field->get_type());

    const StructType* type = StructType::create(*root, field_types, decl);
    decl->set_type(type);

    if (!pScope->add(decl)) {
        Logger::fatal(
            "structure reuses existing name in scope: '" + name.value + "'",
            since(name.loc));
    }

    return decl;
}

EnumDecl* Parser::parse_enum(const Token& name) {
    const Type* underlying = parse_type();

    if (!match(TOKEN_KIND_SET_BRACE)) {
        Logger::fatal(
            "expected '{' for enum declaration after type identifier",
            since(name.loc));
    }

    next(); // '{'

    EnumDecl* decl = new EnumDecl(
        since(name.loc),
        name.value,
        {},
        nullptr,
        {});

    const EnumType* type = EnumType::create(*root, underlying, decl);
    decl->set_type(type);
    
    if (!pScope->add(decl)) {
        Logger::fatal(
            "enum reuses existing name in scope: '" + name.value + "'",
            since(name.loc));
    }

    i64 current_value = 0;
    while (!match(TOKEN_KIND_END_BRACE)) {
        if (!match(TOKEN_KIND_IDENTIFIER)) {
            Logger::fatal(
                "expected enum value identifier",
                since(name.loc));
        }

        const Token vname = lexer.last();
        i64 value = current_value;

        next(); // identifier

        if (match(TOKEN_KIND_EQUALS)) {
            next(); // '='

            if (!match(TOKEN_KIND_INTEGER)) {
                Logger::fatal(
                    "expected integer enum value after '='",
                    since(vname.loc));
            }

            value = std::stol(lexer.last().value);
            current_value = value + 1;
            next(); // value
        } else {
            current_value++;
        }

        EnumValueDecl* value_decl = new EnumValueDecl(
            Span(vname.loc),
            vname.value,
            {},
            type,
            value);

        if (!pScope->add(value_decl)) {
            Logger::fatal(
                "enum value reuses existing name in scope: '" + vname.value + "'",
                since(vname.loc));
        }

        decl->append_value(value_decl);

        if (match(TOKEN_KIND_END_BRACE))
            break;

        if (!match(TOKEN_KIND_COMMA)) {
            Logger::fatal(
                "expected ',' or '}' after enum value",
                since(vname.loc));
        }

        next(); // ','
    } 

    next(); // '}'

    return decl;
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
    Scope* scope = enter_scope();
    std::vector<Stmt*> stmts;
    next(); // '{'

    while (!match(TOKEN_KIND_END_BRACE)) {
        Stmt* stmt = parse_stmt();
        assert(stmt && "could not parse statement");

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
    SourceLocation loc = lexer.last().loc;
    next(); // 'break'

    return new BreakStmt(Span(loc));
}

ContinueStmt* Parser::parse_continue() {
    SourceLocation loc = lexer.last().loc;
    next(); // 'continue'

    return new ContinueStmt(Span(loc));
}

DeclStmt* Parser::parse_decl_stmt() {
    Decl* decl = parse_variable();
    assert(decl && "could not parse variable declaration");

    if (match(TOKEN_KIND_SEMICOLON))
        next(); // ';'

    return new DeclStmt(decl->get_span(), decl);
}

IfStmt* Parser::parse_if() {
    SourceLocation begin = lexer.last().loc;
    next(); // 'if'

    Expr* condition = nullptr;
    Stmt* then_body = nullptr;
    Stmt* else_body = nullptr;

    condition = parse_expr();
    assert(condition && "could not parse 'if' condition");

    then_body = parse_stmt();
    assert(then_body && "could not parse 'if' then body");

    if (match("else")) {
        next(); // 'else'
        else_body = parse_stmt();
        assert(else_body && "could not parse 'if' else body");
    }

    return new IfStmt(since(begin), condition, then_body, else_body);
}

WhileStmt* Parser::parse_while() {
    SourceLocation begin = lexer.last().loc;
    next(); // 'while'

    Expr* cond = nullptr;
    Stmt* body = nullptr;

    cond = parse_expr();
    assert(cond && "could not parse 'while' condition");

    body = parse_stmt();
    assert(body && "could not parse 'while' body");

    return new WhileStmt(since(begin), cond, body);
}

RetStmt* Parser::parse_ret() {
    SourceLocation begin = lexer.last().loc;
    next(); // 'ret'

    Expr* expr = nullptr;
    if (!match(TOKEN_KIND_SEMICOLON)) {
        expr = parse_expr();
        if (!expr)
            Logger::fatal("expected expression after 'ret'", since(begin));
    }

    if (!match(TOKEN_KIND_SEMICOLON))
        Logger::fatal("expected ';' after 'ret' statement", since(begin));

    SourceLocation end = lexer.last().loc;
    next(); // ';'
    return new RetStmt(Span(begin, end), expr);
}

Expr* Parser::parse_expr() {
    Expr* base = parse_unary_prefix();
    assert(base && "could not parse expression base");
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
    else if (match("null"))
        return parse_null();
    else if (match("true") || match("false"))
        return parse_bool();
    else if (match("sizeof"))
        return parse_sizeof();
    
    next(); // identifier

    if (match(TOKEN_KIND_SET_PAREN))
        return parse_call();
    else
        return parse_ref();
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
        assert(right && "could not parse primary binary right operand");

        i32 next_prec = binop_precedence(lexer.last().kind);
        if (tok_prec < next_prec) {
            right = parse_binary(right, precedence + 1);
            assert(right && "could not parse secondary binary right operand");
        }

        pBase = new BinaryExpr(
            Span(pBase->get_span().begin, right->get_span().end),
            nullptr,
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
        assert(base && "could not parse prefix unary operand");

        return new UnaryExpr(
            Span(begin, base->get_span().end),
            nullptr,
            op,
            base,
            false);
    } else {
        return parse_unary_postfix();
    }
}

Expr* Parser::parse_unary_postfix() {
    Expr* expr = parse_primary();
    assert(expr && "could not parse primary expression");

    while (true) {
        SourceLocation begin = lexer.last().loc;
        UnaryExpr::Operator op = unop(lexer.last().kind);
        if (UnaryExpr::is_postfix(op)) {
            next(); // op
            expr = new UnaryExpr(
                Span(begin),
                nullptr,
                op,
                expr,
                true);
        } else if (match(TOKEN_KIND_SET_BRACKET)) {
            // Token is not an operator, but a subscript '[' ... ']'
            next(); // '['

            Expr* index = parse_expr();
            assert(index && "could not parse subscript index expression");

            if (!match(TOKEN_KIND_END_BRACKET))
                Logger::fatal("expected ']' after subscript expression", since(begin));

            next(); // ']'
            expr = new SubscriptExpr(
                since(begin),
                nullptr,
                expr,
                index);
        } else if (match(TOKEN_KIND_DOT)) {
            // Token is not an operator, but a member access '.'
            next(); // '.'

            if (!match(TOKEN_KIND_IDENTIFIER))
                Logger::fatal("expected struct member after '.' operator", since(begin));

            std::string member = lexer.last().value;
            next(); // identifier

            expr = new MemberExpr(
                since(begin),
                nullptr,
                member,
                expr);
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
    next(); // '...'
    return character;
}

StringLiteral* Parser::parse_string() {
    StringLiteral* string = new StringLiteral(
        Span(lexer.last().loc),
        PointerType::get(*root, root->get_char_type()),
        lexer.last().value
    );
    next(); // "..."
    return string;
}

NullLiteral* Parser::parse_null() {
    NullLiteral* null = new NullLiteral(
        Span(lexer.last().loc),
        PointerType::get(*root, root->get_void_type())
    );
    next(); // 'null'
    return null;
}

CastExpr* Parser::parse_cast() {
    SourceLocation begin = lexer.last().loc;
    next(); // 'cast'

    const Type* type = nullptr;
    Expr* expr = nullptr;
    
    if (!match(TOKEN_KIND_LEFT))
        Logger::fatal("expected '<' after 'cast' keyword", since(begin));

    next(); // '<'

    type = parse_type();

    if (!match(TOKEN_KIND_RIGHT))
        Logger::fatal("expected '>' after cast type", since(begin));

    next(); // '>'

    if (!match(TOKEN_KIND_SET_PAREN))
        Logger::fatal("expected '(' after cast type", since(begin));
    
    next(); // '('

    expr = parse_expr();
    assert(expr && "could not parse cast expression");

    if (!match(TOKEN_KIND_END_PAREN))
        Logger::fatal("expected ')' after cast expression", since(begin));

    SourceLocation end = lexer.last().loc;
    next(); // ')'

    return new CastExpr(
        Span(begin, end), 
        type, 
        expr);
}

ParenExpr* Parser::parse_paren() {
    SourceLocation begin = lexer.last().loc;
    next(); // '('

    Expr* expr = parse_expr();
    if (!expr)
        Logger::fatal("expected exprssion after '('", since(begin));

    if (!match(TOKEN_KIND_END_PAREN))
        Logger::fatal("expected ')' to enclose parentheses", since(begin));

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

    if (!match(TOKEN_KIND_SET_PAREN))
        Logger::fatal("expected '(' after 'sizeof' operator", since(begin));

    next(); // '('

    const Type* type = parse_type();

    if (!match(TOKEN_KIND_END_PAREN))
        Logger::fatal("expected ')' after 'sizeof' operator type", since(begin));

    SourceLocation end = lexer.last().loc;
    next(); // ')'

    return new SizeofExpr(
        Span(begin, end),
        root->get_ui64_type(),
        type
    );
}

ReferenceExpr* Parser::parse_ref() {
    const Token& name = lexer.last(1);
    Decl* decl = pScope->get(name.value);
    return new ReferenceExpr(
        Span(name.loc), 
        nullptr, 
        name.value);
}

CallExpr* Parser::parse_call() {
    const Token& callee = lexer.last(1);
    next(); // '('

    std::vector<Expr*> args;

    while (!match(TOKEN_KIND_END_PAREN)) {
        Expr* arg = parse_expr();
        assert(arg && "could not parse call argument expression");

        args.push_back(arg);

        if (match(TOKEN_KIND_END_PAREN)) break;

        if (!match(TOKEN_KIND_COMMA))
            Logger::fatal("expected ',' after function call argument", since(callee.loc));

        next(); // ','
    }

    SourceLocation end = lexer.last().loc;
    next(); // ')'
    return new CallExpr(
        Span(callee.loc, end), 
        nullptr, 
        callee.value, 
        args);
}
