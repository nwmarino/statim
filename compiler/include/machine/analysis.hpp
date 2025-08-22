#ifndef STATIM_MACHINE_ANALYSIS_HPP_
#define STATIM_MACHINE_ANALYSIS_HPP_

#include "bytecode.hpp"
#include "machine/target.hpp"

namespace stm {

/// Analysis pass to lower bytecode to a target-dependent representation.
class FrameMachineAnalysis final {
    Frame& m_frame;
    Target* m_target;

public:
    FrameMachineAnalysis(Frame& frame, Target* target);

    FrameMachineAnalysis(const FrameMachineAnalysis&) = delete;
    FrameMachineAnalysis& operator = (const FrameMachineAnalysis&) = delete;

    void run();
};

/// Analysis pass to do liveness analysis, register allocation, etc.
class FunctionRegisterAnalysis final {
    FrameMachineInfo& m_frame;

public:
    FunctionRegisterAnalysis(FrameMachineInfo& frame);
    
    FunctionRegisterAnalysis(const FunctionRegisterAnalysis&) = delete;
    FunctionRegisterAnalysis& operator = (const FunctionRegisterAnalysis&) = delete;

    void run();
};

/// Frame pass to dump machine code with optimizer details.
class FrameMachinePrinter final {
    FrameMachineInfo& m_frame;

public:
    FrameMachinePrinter(FrameMachineInfo& frame);
    
    FrameMachinePrinter(const FrameMachinePrinter&) = delete;
    FrameMachinePrinter& operator = (const FrameMachinePrinter&) = delete;

    void run(std::ostream& os);
};

/// Frame pass to emit final machine code.
class FrameMachineAsmWriter final {
    FrameMachineInfo& m_frame;

public:
    FrameMachineAsmWriter(FrameMachineInfo& frame);
    
    FrameMachineAsmWriter(const FrameMachineAsmWriter&) = delete;
    FrameMachineAsmWriter& operator = (const FrameMachineAsmWriter&) = delete;

    void run(std::ostream& os);
};;

} // namespace stm

#endif // STATIM_MACHINE_ANALYSIS_HPP_
