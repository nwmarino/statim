#ifndef STATIM_MACHINE_ANALYSIS_HPP_
#define STATIM_MACHINE_ANALYSIS_HPP_

#include "bytecode.hpp"
#include "machine/target.hpp"

namespace stm {

/// Analysis pass to lower bytecode to a target-dependent representation.
class FrameMachineAnalysis final {
    Frame& m_frame;
    Target m_target;

public:
    FrameMachineAnalysis(Frame& frame, Target target);

    FrameMachineAnalysis(const FrameMachineAnalysis&) = delete;
    FrameMachineAnalysis& operator = (const FrameMachineAnalysis&) = delete;

    void run();
};

} // namespace stm

#endif // STATIM_MACHINE_ANALYSIS_HPP_
