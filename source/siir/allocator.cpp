#include "siir/allocator.hpp"

using namespace stm;
using namespace stm::siir;

bool RegisterAllocator::is_available(MachineRegister reg, u32 start, 
                                     u32 end) const {
    /// TODO: This ends up being very slow, since it covers all ranges in a
    /// function. Should be optimized, i.e. keeping a set of non-active but
    /// overlapping ranges, which active is a subset of.
    for (auto& range : m_ranges) {
        // For each range within the function, if it allocates |reg| and 
        // overlaps with [start, end], then |reg| is considered unavailable.

        if (range.alloc == reg && range.overlaps(start, end))
            return false;
    }

    return true;
}

void RegisterAllocator::expire_intervals(LiveRange& curr) {
    for (auto it = m_active.begin(); it != m_active.end(); ) {
        if (it->end < curr.start)
            m_active.erase(it);
        else
            ++it;
    }
}

void RegisterAllocator::assign_register(LiveRange& range) {
    const auto& set = m_pool.regs.at(range.cls);
    for (const auto& reg : set.regs) {
        assert(MachineRegister(reg).is_physical() && 
            "expected physical register!");

        if (is_available(reg, range.start, range.end)) {
            range.alloc = reg;
            break;
        }
    }

    assert(range.alloc != MachineRegister::NoRegister &&
        "failed to allocate register, spilling not implemented!");
}

RegisterAllocator::RegisterAllocator(MachineFunction& function, 
                                     const TargetRegisters& pool,
                                     std::vector<LiveRange>& ranges)
    : m_function(function), m_pool(pool), m_ranges(ranges) {}

void RegisterAllocator::run() {
    for (auto& range : m_ranges) {
        expire_intervals(range);

        if (range.alloc == MachineRegister::NoRegister)
            assign_register(range);
        
        m_active.push_back(range);
    }
}
