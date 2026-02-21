//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_RUNE_H_
#define LACE_RUNE_H_

#include <cstdint>
#include <vector>

namespace lace {

class Expr;
class Rune;

class Rune final {
public:
    /// The different kinds of runes.
    enum class Kind : uint32_t {
        Abort,
        Intrinsic,
        Public,
        Private,
        Unreachable,
    };

private:
    const Kind m_kind;
    std::vector<Expr*> m_args;

public:
    Rune(Kind kind, const std::vector<Expr*>& args = {}) 
      : m_kind(kind), m_args(args) {}

    ~Rune();

    Rune(const Rune&) = delete;
    void operator=(const Rune&) = delete;

    Rune(Rune&&) noexcept = delete;
    void operator=(Rune&&) noexcept = delete;

    /// Returns the kind of this rune.
    Kind kind() const { return m_kind; }

    /// Test if this rune is of the given |kind|.
    bool has_kind(Kind kind) const { return m_kind == kind; }

    /// Set the argument list of this rune to |args|.
    void set_args(const std::vector<Expr*>& args) { m_args = args; }
    
    /// Returns the argument list of this rune.
    const std::vector<Expr*>& args() const { return m_args; }
    std::vector<Expr*>& args() { return m_args; }

    /// Returns the number of arguments to this rune.
    uint32_t num_args() const { return m_args.size(); }

    /// Test if this rune has any arguments.
    bool has_args() const { return !m_args.empty(); }
};

} // namespace lace

#endif // LACE_RUNE_H_
