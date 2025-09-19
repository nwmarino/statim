#ifndef STATIM_SIIR_ALLOCATOR_H_
#define STATIM_SIIR_ALLOCATOR_H_

#include "siir/machine_function.hpp"
#include "siir/machine_register.hpp"
#include "siir/target.hpp"

#include <unordered_map>
#include <vector>

namespace stm::siir {

struct RegisterSet final {
    RegisterClass cls;
    std::vector<u32> regs;
};

struct TargetRegisters final {
    std::unordered_map<RegisterClass, RegisterSet> regs;
};

/// Represents the positional range in which a register is live.
struct LiveRange final {

    /// The register that this range represents, pre-allocations. For ranges
    /// made for physical registers, this still represents the physical
    /// register.
    MachineRegister reg;

    /// The physical register that was allocated over this range.
    MachineRegister alloc;

    /// The start and end positions of this range.
    u32 start, end;

    /// The desired register class for this range.
    RegisterClass cls;

    /// If true, then this range is considered dead and should no longer be
    /// extended.
    bool killed;

    /// Returns true if this range in any way overlaps with the given position.
    bool overlaps(u32 pos) const {
        return this->start < pos && pos < this->end;
    }

    /// Returns true if this range in any way overlaps with the bounds 
    /// [start, end].
    bool overlaps(u32 start, u32 end) const {
        return this->start < end && this->end > start;
    }
};

class RegisterAllocator {
    const TargetRegisters& m_pool;
    MachineFunction& m_function;
    
    std::vector<LiveRange>& m_ranges;
    std::vector<LiveRange> m_active = {};
    
    bool active_contains(MachineRegister reg) const;
    bool is_available(MachineRegister reg, u32 start, u32 end) const;

    void expire_intervals(LiveRange& curr);
    void assign_register(LiveRange& range);

public:
    RegisterAllocator(MachineFunction& function, const TargetRegisters& pool,
                      std::vector<LiveRange>& ranges);

    RegisterAllocator(const RegisterAllocator&) = delete;
    RegisterAllocator& operator = (const RegisterAllocator&) = delete;

    ~RegisterAllocator() = default;

    void run();
};

} // namespace stm::siir

#endif // STATIM_SIIR_ALLOCATOR_H_
