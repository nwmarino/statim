#ifndef STATIM_SIIR_CONSTANT_HPP_
#define STATIM_SIIR_CONSTANT_HPP_

#include "siir/user.hpp"

namespace stm {

class Constant : public User {

};

class ConstantInt final : public Constant {

};

class ConstantFP final : public Constant {

};

class ConstantNull final : public Constant {

};

class BlockAddress final : public Constant {

};

class ConstantAggregate : public Constant {

};

class ConstantArray final : public ConstantAggregate {

};

class ConstantStruct final : public ConstantAggregate {

};

class ConstantExpr : public Constant {

};

} // namespace stm

#endif // STATIM_SIIR_CONSTANT_HPP_
