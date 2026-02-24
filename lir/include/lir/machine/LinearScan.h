//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_LINEAR_SCAN_H_
#define LIR_LINEAR_SCAN_H_

#include "lir/machine/MachineFunction.h"
#include "lir/machine/MachineOp.h"
#include "lir/machine/RegisterAllocator.h"

#include <vector>

namespace lir {

class LinearScan final {
    MachineFunction& m_func;
    std::vector<LiveRange>& m_ranges;
    uint32_t m_pos = 0;

public:
    LinearScan(MachineFunction& func, std::vector<LiveRange>& ranges);

    LinearScan(const LinearScan&) = delete;
    void operator=(const LinearScan&) = delete;

    LinearScan(LinearScan&&) noexcept = delete;
    void operator=(LinearScan&&) noexcept = delete;

    void run();

private:
    /// Process the given |op| and its operands.
    void process(MachineOp* op);

    /// Update the live range for the given |reg|.
    LiveRange& updateRange(Register reg, RegisterClass cls, uint32_t pos);
};

} // namespace lir

#endif // LIR_LINEAR_SCAN_H_
