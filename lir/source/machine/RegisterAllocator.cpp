//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/AMD64.hpp"
#include "lir/machine/Register.hpp"
#include "lir/machine/RegisterAllocator.h"

using namespace lir;

RegisterAllocator::RegisterAllocator(MachineFunction& func, std::vector<LiveRange>& ranges)
  : m_func(func), m_ranges(ranges) {
    
    m_pool[RegisterClass::GeneralPurpose] = {
        RAX, RCX, RDX, RSI, RDX, 
        R8, R9, R10, R11, 
        R12, R13, R14, R15
    };

    m_pool[RegisterClass::FloatingPoint] = {
        XMM0, XMM1, XMM2, XMM3, 
        XMM4, XMM5, XMM6, XMM7, 
        XMM8, XMM9, XMM10, XMM11,
        XMM12, XMM13, XMM14, XMM15,
    };
}

void RegisterAllocator::run() {
    for (LiveRange& range : m_ranges) {
        expireIntervals(range);

        if (range.alloc == Register::NO_REGISTER)
            allocRegister(range);

        m_active.push_back(range);
    }
}

bool RegisterAllocator::isAvailable(Register reg, uint32_t start, uint32_t end) const {
    // @Todo: optimize by keeping a set of non-active, overlapping ranges, which |m_active| is a
    // subset of, or something similar.

    // For each range in the function, check if the range uses the given physical |reg|, and see
    // if it conflicts with [start, end].
    for (const LiveRange& range : m_ranges) {
        if (range.alloc == reg && range.overlaps(start, end))
            return false;
    }

    return true;
}

void RegisterAllocator::expireIntervals(LiveRange& range) {
    // For each live range currently active, if it does not overlap with |range|, then expire it.  
    for (auto it = m_active.begin(); it != m_active.end(); ) {
        if (it->end < range.start) {
            m_active.erase(it);
        } else {
            ++it;
        }
    }
}

void RegisterAllocator::allocRegister(LiveRange& range) {
    const auto& set = m_pool.at(range.cls);

    // For each register in the pool with the same class as the one needed by |range|, if it is
    // available, then allocate it to |range|.
    for (const auto& reg : set) {
        if (isAvailable(reg, range.start, range.end)) {
            range.alloc = reg;
            return;
        }
    }

    // If a register could not be allocated, then the register must be spilled to the stack.
    if (range.alloc == Register::NO_REGISTER) {
        // Spill to the stack.
        assert(false && "not implemented!");
    }
}
