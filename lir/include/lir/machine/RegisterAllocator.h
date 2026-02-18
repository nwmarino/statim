//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_REGISTER_ALLOCATOR_H_
#define LIR_REGISTER_ALLOCATOR_H_

#include "lir/machine/MachineFunction.hpp"
#include "lir/machine/Register.hpp"

#include <unordered_map>
#include <vector>

namespace lir {

struct LiveRange final {
    /// The id of the register this range is for.
    Register reg;

    /// The id of the physical register allocated for this range.
    Register alloc;

    /// The register class to allocate this register to.
    RegisterClass cls;

    /// If this range has explicitly expired.
    bool expired;
    
    /// The start and end positions of this range.
    uint32_t start, end;

    /// Test if this range overlaps with the given |position|.
    bool overlaps(uint32_t position) const {
        return this->start < position && position < this->end;
    }

    /// Test if this range overlaps with the range [start, end].
    bool overlaps(uint32_t start, uint32_t end) const {
        return this->start < end && this->end > start;
    }
};

class RegisterAllocator final {
    using Pool = std::unordered_map<RegisterClass, std::vector<uint32_t>>;

    MachineFunction& m_func;
    std::vector<LiveRange>& m_ranges;
    std::vector<LiveRange> m_active = {};    
    Pool m_pool = {};

public:
    RegisterAllocator(MachineFunction& func, std::vector<LiveRange>& ranges);
    
    RegisterAllocator(const RegisterAllocator&) = delete;
    void operator=(const RegisterAllocator&) = delete;

    RegisterAllocator(RegisterAllocator&&) noexcept = delete;
    void operator=(RegisterAllocator&&) noexcept = delete;

    void run();

private:
    /// Test if the given physical |reg| is available between the |start| and |end| positions.
    bool isAvailable(Register reg, uint32_t start, uint32_t end) const;

    /// Expire any intervals that are no longer active at the point of |range|.
    void expireIntervals(LiveRange& range);

    /// Allocate a register for the given live |range|.
    void allocRegister(LiveRange& range);
};

} // namespace lir

#endif // LIR_REGISTER_ALLOCATOR_H_
