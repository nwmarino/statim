#ifndef STATIM_SIIR_CONSTANT_HPP_
#define STATIM_SIIR_CONSTANT_HPP_

#include "siir/user.hpp"
#include <initializer_list>

namespace stm {

namespace siir {

class BasicBlock;
class CFG;

class Constant : public User {
protected:
    Constant() = default;

    Constant(std::initializer_list<Value*> ops, const Type* type, 
             const std::string& name = "") 
        : User(ops, type, name) {}
    

public:
    virtual ~Constant() = default;
};

class ConstantInt final : public Constant {
    friend class Context;

    i64 m_value;

    ConstantInt(i64 value, const Type* type);

public:
    static Constant* get_true(CFG& cfg);
    static Constant* get_false(CFG& cfg);
    static Constant* get_zero(CFG& cfg, const Type* type);
    static Constant* get_one(CFG& cfg, const Type* type);
    static Constant* get(CFG& cfg, const Type* type, i64 value);

    i64 get_value() const { return m_value; }

    void print(std::ostream& os) const override;
};

class ConstantFP final : public Constant {
    friend class Context;

    f64 m_value;

    ConstantFP(f64 value, const Type* type);

public:
    static Constant* get_zero(CFG& cfg, const Type* type);
    static Constant* get_one(CFG& cfg, const Type* type);
    static Constant* get(CFG& cfg, const Type* type, f64 value);

    f64 get_value() const { return m_value; }

    void print(std::ostream& os) const override;
};

class ConstantNull final : public Constant {
    friend class Context;
    
    ConstantNull(const Type* type) : Constant({}, type) {}

public:
    static Constant* get(CFG& cfg, const Type* type);

    void print(std::ostream& os) const override;
};

class BlockAddress final : public Constant {
    friend class Context;

    BasicBlock* m_block;

    BlockAddress(BasicBlock* blk) : Constant(), m_block(blk) {}

public:
    static Constant* get(CFG& cfg, BasicBlock* blk);

    const BasicBlock* get_block() const { return m_block; }
    BasicBlock* get_block() { return m_block; }

    void print(std::ostream& os) const override;
};

/*
class ConstantAggregate : public Constant {

};

class ConstantArray final : public ConstantAggregate {
    std::vector<Constant*> m_values;
};

class ConstantStruct final : public ConstantAggregate {
    std::vector<Constant*> m_values;
};

class ConstantExpr : public Constant {

};
*/

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_CONSTANT_HPP_
