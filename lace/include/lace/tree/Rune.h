//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_RUNE_H_
#define LACE_RUNE_H_

//
//  This header file defines the Rune type, which is a representation of the language construct of 
//  the same name that gives way to many of the meta-programming abilities in the language.
//

#include "lace/core/Common.h"

#include <cstdint>
#include <vector>

namespace lace {

class Expr;
class Rune;

class Rune final {
public:
    /// The different ty[es] of runes.
    enum Type : uint32_t {
        Abort,
        Intrinsic,
        Public,
        Private,
        Unreachable,
    };

private:
    const Type m_type;
    std::vector<Expr*> m_args;

public:
    Rune(Type type, const std::vector<Expr*>& args = {}) : m_type(type), m_args(args) {}

    ~Rune();

    Rune(const Rune&) = delete;
    void operator=(const Rune&) = delete;

    Rune(Rune&&) noexcept = delete;
    void operator=(Rune&&) noexcept = delete;

    /// Returns the type of this rune.
    Type getType() const { return m_type; }

    /// Test if this rune is of the given |type|.
    Result hasType(Type type) const { return m_type == type; }

    /// Sets the argument list of this rune to |args|.
    void setArgs(const std::vector<Expr*>& args) { m_args = args; }
    
    /// Returns the argument list to this rune.
    const std::vector<Expr*>& getArgs() const { return m_args; }
    std::vector<Expr*>& getArgs() { return m_args; }

    /// Returns the number of arguments to this rune.
    uint32_t numArgs() const { return m_args.size(); }

    /// Test if this rune has any arguments.
    Result hasArgs() const { return !m_args.empty(); }
};

} // namespace lace

#endif // LOVELACE_RUNE_H_
