#ifndef STATIM_SIIR_MACHINE_FUNCTION_H_
#define STATIM_SIIR_MACHINE_FUNCTION_H_

#include "siir/target.hpp"
#include "siir/machine_basicblock.hpp"
#include "siir/machine_register.hpp"
#include "types/types.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace stm::siir {

class Function;
class MachineInst;

/// An entry in the stack frame of a function.
///
/// This databag effectively reverses space on the stack of a function for a 
/// local in the SIIR equivelant function.
struct FunctionStackEntry final {
    /// The offset of this entry in the stack.
    i32 offset;

    /// The number of bytes this entry reserves.
    u32 size;

    /// The desired alignment of this entry.
    u32 align;
};

/// Information about the stack of a machine function.
struct FunctionStackInfo final {
    std::vector<FunctionStackEntry> entries;

    /// Returns the number of entries in this stack.
    u32 num_entries() const { return entries.size(); }

    /// Returns the size of the stack in bytes, without any alignment.
    u32 size() const {
        if (entries.empty())
            return 0;

        return entries.back().offset + entries.back().size;
    }
};

/// Information about a virtual register.
struct VRegInfo final {
    /// The desired class for a virtual register post-allocation.
    RegisterClass cls = GeneralPurpose;

    /// Starting point of a virtual registers' lifespan (definition).
    u32 start;

    /// End point of a virtual registers' lifespan (last use).
    u32 end;

    /// The resulting allocation of a virtual register.
    u32 alloc = MachineRegister::NoRegister;
};

/// Information about the registers used by a machine function.
struct FunctionRegisterInfo final {
    std::unordered_map<u32, VRegInfo> vregs;
};

/// Represents a machine function, derived from a bytecode function.
class MachineFunction final {
    friend class FunctionRegisterAnalysis;

    /// Internal information about this function.
    FunctionStackInfo m_stack;
    FunctionRegisterInfo m_regi;

    /// The bytecode function this derives from.
    const Function* m_fn;
    const siir::Target& m_target;

    /// Links to the first and last basic blocks in this function. 
    MachineBasicBlock* m_front = nullptr;
    MachineBasicBlock* m_back = nullptr;

public:
    MachineFunction(const Function* fn, const siir::Target& target);

    MachineFunction(const MachineFunction&) = delete;
    MachineFunction& operator = (const MachineFunction&) = delete;

    ~MachineFunction();

    /// Returns the SIIR function that this function derives from.
    const Function* get_function() const { return m_fn; }
    
    /// Returns the target that this function was compiled for.
    const siir::Target& get_target() const { return m_target; }

    /// Returns the name of this function, as it was defined in the SIIR.
    const std::string& get_name() const;

    const FunctionRegisterInfo& get_register_info() const { return m_regi; }
    FunctionRegisterInfo& get_register_info() { return m_regi; }

    const MachineBasicBlock* front() const { return m_front; }
    MachineBasicBlock* front() { return m_front; }

    const MachineBasicBlock* back() const { return m_back; }
    MachineBasicBlock* back() { return m_back; }

    /// Return the basic block at position |idx| in this function.
    const MachineBasicBlock* at(u32 idx) const;
    MachineBasicBlock* at(u32 idx);

    /// Returns the number of basic blocks in this function.
    u32 size() const;

    /// Returns true if this function has no basic blocks.
    bool empty() const { return !m_front; }

    /// Prepend |mbb| to the front of this function.
    void prepend(MachineBasicBlock* mbb);

    /// Append |mbb| to the back of this function.
    void append(MachineBasicBlock* mbb);
};

} // namespace stm::siir

#endif // STATIM_SIIR_MACHINE_FUNCTION_H_
