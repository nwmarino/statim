//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_CALLSITE_ANALYSIS_H_
#define LIR_CALLSITE_ANALYSIS_H_

#include "lir/machine/MachineFunction.h"
#include "lir/machine/RegisterAllocator.h"

#include <vector>

namespace lir {

class CallsiteAnalysis final {
    const Machine& m_mach;
    MachineFunction& m_func;
    std::vector<LiveRange>& m_ranges;

public:
    CallsiteAnalysis(const Machine& mach, MachineFunction& func, std::vector<LiveRange>& ranges);

    ~CallsiteAnalysis() = default;

    CallsiteAnalysis(const CallsiteAnalysis&) = delete;
    void operator=(const CallsiteAnalysis&) = delete;

    CallsiteAnalysis(CallsiteAnalysis&&) noexcept = delete;
    void operator=(CallsiteAnalysis&&) noexcept = delete;

    void run();
};

} // namespace lir

#endif // LIR_CALLSITE_ANALYSIS_H_
