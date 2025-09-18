#ifndef STATIM_SIIR_MACHINE_FUNCTION_H_
#define STATIM_SIIR_MACHINE_FUNCTION_H_

#include "siir/constant.hpp"
#include "siir/local.hpp"
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

    /// The local that defines this entry, if there is one.
    ///
    /// It should be noted that some entries used for spills after instruction
    /// selection do not have a corresponding local.
    const Local* local = nullptr;
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

    u32 alignment() const {
        u32 max_align = 1;
        for (const auto& entry : entries)
            if (entry.align > max_align)
                max_align = entry.align;
        
        u32 size = this->size();
        while (max_align < size)
            max_align += 16;

        if (max_align % 16 != 0)
            max_align += 16 - (max_align % 16);

        return max_align;
    }
};

/// Information about a virtual register.
struct VRegInfo final {
    /// The desired class for a virtual register post-allocation.
    RegisterClass cls = GeneralPurpose;

    /// The resulting allocation of a virtual register.
    MachineRegister alloc = MachineRegister::NoRegister;
};

/// Information about the registers used by a machine function.
struct FunctionRegisterInfo final {
    std::unordered_map<u32, VRegInfo> vregs;
};

/// An entry in the constant pool of a function.
struct FunctionConstantPoolEntry final {
    const Constant* constant;
    u32 align;
};

/// Constants referenced by a function that should be emitted to read-only
/// data sections.
struct FunctionConstantPool final {
    std::vector<FunctionConstantPoolEntry> entries;

    /// Returns the number of entries in this pool.
    u32 num_entries() const { return entries.size(); }

    u32 get_or_create_constant(const Constant* constant, u32 align) {
        u32 idx = 0;
        for (u32 e = entries.size(); idx != e; ++idx) {
            FunctionConstantPoolEntry& entry = entries[idx];
            /// TODO: Optimize comparisons to reduce duplicate constants.
            if (entry.constant == constant && entry.align == align)
                return idx; 
        }

        entries.push_back({ constant, align });
        return idx;
    }
};

/// Represents a machine function, derived from a bytecode function.
class MachineFunction final {
    friend class FunctionRegisterAnalysis;

    /// Internal information about this function.
    FunctionStackInfo m_stack;
    FunctionRegisterInfo m_regi;
    FunctionConstantPool m_pool;

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

    const FunctionStackInfo& get_stack_info() const { return m_stack; }
    FunctionStackInfo& get_stack_info() { return m_stack; }

    const FunctionRegisterInfo& get_register_info() const { return m_regi; }
    FunctionRegisterInfo& get_register_info() { return m_regi; }

    const FunctionConstantPool& get_constant_pool() const { return m_pool; }
    FunctionConstantPool& get_constant_pool() { return m_pool; }

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
