#ifndef STATIM_MACHINE_HPP_
#define STATIM_MACHINE_HPP_

#include "types.hpp"

#include <cassert>
#include <vector>

namespace stm {

class MachineInst;
class MachineFunction;

enum RegisterClass : u32 {
    RC_GPR8,
    RC_GPR16,
    RC_GPR32,
    RC_GRP64,
    RC_FPR32,
    RC_FPR64,
};

class MachineBasicBlock final {

};

class MachineFunction final {

};

class MachineFrame final {

};

} // namespace stm

#endif // STATIM_MACHINE_HPP_
