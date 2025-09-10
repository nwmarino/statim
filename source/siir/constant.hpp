#ifndef STATIM_SIIR_CONSTANT_HPP_
#define STATIM_SIIR_CONSTANT_HPP_

#include "siir/user.hpp"

#include <initializer_list>
#include <string>

namespace stm {
namespace siir {

class BasicBlock;
class CFG;

/// A constant value in the IR.
///
/// Constants are considered users for the sake of constant expressions that
/// are comprised of constant operands.
class Constant : public User {
protected:
    Constant() = default;

    Constant(std::initializer_list<Value*> ops, const Type* type) 
        : User(ops, type) {}

public:
    virtual ~Constant() = default;

    bool is_constant() const override { return true; }

    virtual bool is_aggregate() const { return false; }
};

/// A constant integer literal.
class ConstantInt final : public Constant {
    friend class CFG;

    /// The value of this literal.
    i64 m_value;

    /// Private constructor. To be used by the graph context for pooling.
    ConstantInt(i64 value, const Type* type)
        : Constant({}, type), m_value(value) {}

public:
    ConstantInt(const ConstantInt&) = delete;
    ConstantInt& operator = (const ConstantInt&) = delete;

    /// Get the constant true value, typed with i1.
    static Constant* get_true(CFG& cfg);

    /// Get the constant false value, typed with i1.
    static Constant* get_false(CFG& cfg);

    /// Get a constant zero, typed with |type|.
    static Constant* get_zero(CFG& cfg, const Type* type);

    /// Get a constant one, typed with |type|.
    static Constant* get_one(CFG& cfg, const Type* type);

    /// Get a constant integer with the given value and type.
    static Constant* get(CFG& cfg, const Type* type, i64 value);

    /// Returns the value of this integer literal.
    i64 get_value() const { return m_value; }

    void print(std::ostream& os) const override;
};

/// A constant floating-point literal.
class ConstantFP final : public Constant {
    friend class CFG;

    /// The value of this literal.
    f64 m_value;

    /// Private constructor. To be used by the graph context for pooling.
    ConstantFP(f64 value, const Type* type) 
        : Constant({}, type), m_value(value) {}

public:
    ConstantFP(const ConstantFP&) = delete;
    ConstantFP& operator = (const ConstantFP&) = delete;

    /// Get the constant zero, typed with |type|.
    static Constant* get_zero(CFG& cfg, const Type* type);

    /// Get the constant one, typed with |type|. 
    static Constant* get_one(CFG& cfg, const Type* type);

    /// Get a constant floating point with the given value and type.
    static Constant* get(CFG& cfg, const Type* type, f64 value);

    /// Returns the value of this floating point literal.
    f64 get_value() const { return m_value; }

    void print(std::ostream& os) const override;
};

/// A constant null pointer literal.
class ConstantNull final : public Constant {
    friend class CFG;
    
    /// Private constructor. To be used by the graph context for pooling.
    ConstantNull(const Type* type) : Constant({}, type) {}

public:
    ConstantNull(const ConstantNull&) = delete;
    ConstantNull& operator = (const ConstantNull&) = delete;

    /// Get the constant null for the given type.
    static Constant* get(CFG& cfg, const Type* type);

    void print(std::ostream& os) const override;
};

/// A constant block address, used for branching destinations.
class BlockAddress final : public Constant {
    friend class CFG;

    /// The block this address refers to.
    BasicBlock* m_block;

    /// Private constructor. To be used by the graph context for pooling.
    BlockAddress(BasicBlock* blk) : Constant(), m_block(blk) {}

public:
    /// Get the block address for the given block.
    static Constant* get(CFG& cfg, BasicBlock* blk);

    /// Returns the block that this address refers to.
    const BasicBlock* get_block() const { return m_block; }
    BasicBlock* get_block() { return m_block; }

    void print(std::ostream& os) const override;
};

class ConstantAggregate : public Constant {
public:
    ConstantAggregate(std::initializer_list<Value*> ops, const Type* type)
        : Constant(ops, type) {}

    ConstantAggregate(const ConstantAggregate&) = delete;
    ConstantAggregate& operator = (const ConstantAggregate&) = delete;

    bool is_aggregate() const override { return true; }
};

class ConstantString final : public ConstantAggregate {
    friend class CFG;

    /// The value of this literal.
    const std::string m_value;

    /// Private constructor. To be used by the graph context for pooling.
    ConstantString(const std::string& value, const Type* type) 
        : ConstantAggregate({}, type), m_value(value) {}

public:
    ConstantString(const ConstantString&) = delete;
    ConstantString& operator = (const ConstantString&) = delete;

    /// Get a constant string for |string|.
    static ConstantString* get(CFG& cfg, const std::string& string);

    /// Returns the value of this string constant.
    const std::string& get_value() const { return m_value; }

    void print(std::ostream& os) const override;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_CONSTANT_HPP_
