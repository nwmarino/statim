//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_CONSTANT_H_
#define LOVELACE_IR_CONSTANT_H_

#include "lir/graph/Type.hpp"
#include "lir/graph/User.hpp"

namespace lir {

class BasicBlock;
class CFG;

/// A constant value in the agnostic IR.
///
/// Constants are considered users for the sake of constant expressions that
/// are comprised of constant operands.
class Constant : public User {
protected:
    Constant(Type *type, const std::vector<Value*> &ops = {}) 
      : User(type, ops) {}

public:
    virtual ~Constant() = default;

    Constant(const Constant&) = delete;
    void operator=(const Constant&) = delete;

    Constant(Constant&&) noexcept = delete;
    void operator=(Constant&&) noexcept = delete;

    bool is_constant() const override { return true; }
};

/// A constant integer literal.
class Integer final : public Constant {
    friend class CFG;

    const int64_t m_value;

    Integer(int64_t value, Type *type) : Constant(type), m_value(value) {}

public:
    /// Get the constant true value, typed with `i1`.
    static Integer *get_true(CFG &cfg);

    /// Get the constant false value, typed with `i1`.
    static Integer *get_false(CFG &cfg);

    /// Get a constant zero, with the given |type|.
    static Integer *get_zero(CFG &cfg, Type *type);

    /// Get a constant one, with the given |type|.
    static Integer *get_one(CFG &cfg, Type *type);

    /// Get a constant integer with the given |value| and |type|.
    static Integer *get(CFG &cfg, Type *type, int64_t value);

    int64_t get_value() const { return m_value; }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A constant floating-point literal.
class Float final : public Constant {
    friend class CFG;

    double m_value;

    Float(double value, Type *type) : Constant(type), m_value(value) {}

public:
    /// Get the constant zero, with the given |type|.
    static Float *get_zero(CFG &cfg, Type *type);

    /// Get the constant one, with the given |type|. 
    static Float *get_one(CFG &cfg, Type *type);

    /// Get a constant floating point with the given |value| and |type|.
    static Float *get(CFG &cfg, Type *type, double value);

    double get_value() const { return m_value; }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A constant, typed null pointer literal.
class Null final : public Constant {
    friend class CFG;
    
    Null(Type *type) : Constant(type) {}

public:
    /// Get the null pointer for the given |type|.
    static Null *get(CFG &cfg, Type *type);

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A constant string of ASCII characters.
class String final : public Constant {
    friend class CFG;

    const std::string m_value;

    String(Type *type, const std::string &value) 
      : Constant(type), m_value(value) {}

public:
    /// Get a string constant value for the given |string|.
    static String *get(CFG &cfg, const std::string &string);

    const std::string &get_value() const { return m_value; }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// An static-sized aggregate of constant values.
class Aggregate final : public Constant {
    friend class CFG;

public:
    using Values = std::vector<Constant*>;

private:
    Aggregate(Type *type, const std::vector<Value*> &values)
      : Constant(type, values) {}

public:
    /// Create a new aggregate from the given |values|.
    static Aggregate *get(CFG &cfg, Type *type, const Values &values);

    /// Returns the |i|-th value in this aggregate.
    const Constant *get_value(uint32_t i) const {
        assert(i < num_operands() && "index out of bounds!");
        assert(get_operand(i)->get_value()->is_constant());
        
        return static_cast<const Constant*>(get_operand(i)->get_value());
    }

    Constant *get_value(uint32_t i) {
        assert(i < num_operands() && "index out of bounds!");
        assert(get_operand(i)->get_value()->is_constant());
        
        return static_cast<Constant*>(get_operand(i)->get_value());
    }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

} // namespace lir

#endif // LOVELACE_IR_CONSTANT_H_
