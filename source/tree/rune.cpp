#include "tree/rune.hpp"

using namespace stm;

bool Rune::is_decorator(Kind kind) {
    switch (kind) {
    case ABI:
    case Alignas:
    case Deprecated:
    case Dump:
    case Inline:
    case Intrinsic:
    case NoDiscard:
    case NoOptimize:
    case NoReturn:
    case NoScope:
    case Packed:
    case Public:
    case Private:
    case Unsafe:
        return true;
    default:
        return false;
    }
}

bool Rune::is_value(Kind kind) {
    switch (kind) {
    case Comptime:
    case Path:
        return true;
    default:
        return false;
    }
}

bool Rune::is_statement(Kind kind) {
    switch (kind) {
    case Abort:
    case Asm:
    case Assert:
    case If:
    case Print:
    case Println:
    case Write:
    case Writeln:
        return true;
    default:
        return false;
    }
}

bool Rune::accepts_args(Kind kind) {
    switch (kind) {
    case ABI:
    case Alignas:
    case Assert:
    case Print:
    case Println:
    case Write:
    case Writeln:
        return true;
    default:
        return false;
    }
}

Rune::Kind Rune::from_string(const std::string& str) {
    if (str == "abi") return ABI;
    if (str == "alignas") return Alignas;
    if (str == "deprecated") return Deprecated;
    if (str == "dump") return Dump;
    if (str == "inline") return Inline;
    if (str == "intrinsic") return Intrinsic;
    if (str == "no_discard") return NoDiscard;
    if (str == "no_optimize") return NoOptimize;
    if (str == "no_return") return NoReturn;
    if (str == "no_scope") return NoScope;
    if (str == "packed") return Packed;
    if (str == "public") return Public;
    if (str == "private") return Private;
    if (str == "unsafe") return Unsafe;
    if (str == "comptime") return Comptime;
    if (str == "path") return Path;
    if (str == "abort") return Abort;
    if (str == "asm") return Asm;
    if (str == "assert") return Assert;
    if (str == "if") return If;
    if (str == "print") return Print;
    if (str == "println") return Println;
    if (str == "write") return Write;
    if (str == "writeln") return Writeln;
    return Unknown;
}

std::string Rune::to_string(Rune::Kind kind) {
    switch (kind) {
    case Unknown:
        return "unknown";
    case ABI:
        return "abi";
    case Alignas:
        return "alignas";
    case Deprecated:
        return "deprecated";
    case Dump:
        return "dump";
    case Inline:
        return "inline";
    case Intrinsic:
        return "intrinsic";
    case NoDiscard:
        return "no_discard";
    case NoOptimize:
        return "no_optimize";
    case NoReturn:
        return "no_return";
    case NoScope:
        return "no_scope";
    case Packed:
        return "packed";
    case Public:
        return "public";
    case Private:
        return "private";
    case Unsafe:
        return "unsafe";
    case Comptime:
        return "comptime";
    case Path:
        return "path";
    case Abort:
        return "abort";
    case Asm:
        return "asm";
    case Assert:
        return "assert";
    case If:
        return "if";
    case Print:
        return "print";
    case Println:
        return "println";
    case Write:
        return "write";
    case Writeln:
        return "writeln";
    }
}

Rune::Rune(Kind kind, const std::vector<Expr*>& args) 
    : m_kind(kind), m_args(args) {}

Rune::~Rune() {
    for (auto* arg : m_args) delete arg;
    m_args.clear();
}

RuneStmt::RuneStmt(const Span& span, Rune* rune)
    : Stmt(span), m_rune(rune) {}

RuneStmt::~RuneStmt() {
    delete m_rune;
};

RuneExpr::RuneExpr(const Span& span, const Type* type, Rune* rune)
    : Expr(span, type), m_rune(rune) {}

RuneExpr::~RuneExpr() {
    delete m_rune;
}
