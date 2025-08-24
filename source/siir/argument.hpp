#ifndef STATIM_SIIR_ARGUMENT_HPP_
#define STATIM_SIIR_ARGUMENT_HPP_

#include "siir/value.hpp"
#include "types/types.hpp"

namespace stm {

class BasicBlock;
class Function;

class Argument : public Value {
    u32 m_index;
};

class BlockArgument final : public Argument {
    BasicBlock* m_parent;
};

class FunctionArgument final : public Argument {
    Function* m_parent;
};

} // namespace stm

#endif // STATIM_SIIR_ARGUMENT_HPP_
