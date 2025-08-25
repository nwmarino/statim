#ifndef STATIM_SIIR_CONSTANT_HPP_
#define STATIM_SIIR_CONSTANT_HPP_

#include "siir/user.hpp"

namespace stm {

namespace siir {

class BasicBlock;

class Constant : public User {

};

class ConstantInt final : public Constant {
    i64 m_value;

    ConstantInt(i64 value, const Type* type);
};

class ConstantFP final : public Constant {
    f64 m_value;

    ConstantFP(f64 value, const Type* type);
};

class ConstantNull final : public Constant {
    ConstantNull(const Type* type);
};

class BlockAddress final : public Constant {
    BasicBlock* m_block;

    BlockAddress(BasicBlock* block);
};

class ConstantAggregate : public Constant {

};

class ConstantArray final : public ConstantAggregate {

};

class ConstantStruct final : public ConstantAggregate {

};

class ConstantExpr : public Constant {

};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_CONSTANT_HPP_
