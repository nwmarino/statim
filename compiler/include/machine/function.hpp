#ifndef STATIM_MACHINE_FUNCTION_HPP_
#define STATIM_MACHINE_FUNCTION_HPP_

#include "machine/basicblock.hpp"
#include "machine/register.hpp"
#include "types.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace stm {

class Function;
class MachineInst;

/// An entry in a functions stack frame.
struct FunctionStackEntry final {
    i32 offset;
    u32 size;
    u32 align;
};

/// Information about the stack of a machine function.
struct FunctionStackInfo final {
    std::vector<FunctionStackEntry> entries;

    u32 size() const {
        if (entries.empty())
            return 0;

        return entries.back().offset + entries.back().size;
    }
};

/// Information about a virtual register.
struct VRegInfo final {
    /// The desired class for a virtual register post-allocation.
    RegisterClass cls;

    /// Starting point of a virtual registers' lifespan (definition).
    u32 start;

    /// End point of a virtual registers' lifespan (last use).
    u32 end;

    /// The resulting allocation of a virtual register.
    u32 alloc = Register::NoRegister;
};

/// Information about the registers used by a machine function.
struct FunctionRegisterInfo final {
    std::unordered_map<u32, VRegInfo> vregs;
};

/// Represents a machine function, derived from a bytecode function.
class MachineFunction final {
    /// Internal information about this function.
    FunctionStackInfo m_stack;
    FunctionRegisterInfo m_regi;

    /// The bytecode function this derives from.
    const Function* m_fn;

    /// Links to the first and last basic blocks in this function. 
    MachineBasicBlock* m_front;
    MachineBasicBlock* m_back;

public:
    MachineFunction(const Function* fn);
    ~MachineFunction();

    /// \returns The bytecode function that this function derives from.
    const Function* get_function() const { return m_fn; }

    /// \returns The name of this function, as it was defined in the bytecode.
    const std::string& get_name() const;

    const MachineBasicBlock* front() const { return m_front; }
    MachineBasicBlock* front() { return m_front; }

    const MachineBasicBlock* back() const { return m_back; }
    MachineBasicBlock* back() { return m_back; }

    void print(std::ostream& os) const;
};

} // namespace stm

#endif // STATIM_MACHINE_FUNCTION_HPP_
