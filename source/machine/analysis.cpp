#include "machine/analysis.hpp"
#include "bytecode.hpp"
#include "machine/basicblock.hpp"
#include "machine/function.hpp"
#include "target/amd64.hpp"

using namespace stm;

FrameMachineAnalysis::FrameMachineAnalysis(Frame& frame, Target* target)
    : m_frame(frame), m_target(target) {}

void FrameMachineAnalysis::run() {
    FrameMachineInfo& info = m_frame.get_machine_info();
    info.m_target = m_target;
    info.m_functions = {};

    for (auto function : m_frame.functions()) {
        MachineFunction* mf = new MachineFunction(function, *m_target);
        info.m_functions.emplace(function->get_name(), mf);

        for (auto curr = function->front(); curr; curr = curr->next())
            new MachineBasicBlock(curr, mf);

        switch (m_target->arch()) {
        case Target::amd64:
            amd64::InstSelection isel { mf };
            isel.run();
        }
    }
}

FunctionRegisterAnalysis::FunctionRegisterAnalysis(FrameMachineInfo& frame)
    : m_frame(frame) {}

void FunctionRegisterAnalysis::run() {
    for (auto function : m_frame.functions()) {
        FunctionRegisterInfo& info = function->m_regi;

    }
}

FrameMachinePrinter::FrameMachinePrinter(FrameMachineInfo& frame)
    : m_frame(frame) {}

void FrameMachinePrinter::run(std::ostream& os) {
    switch (m_frame.get_target()->arch()) {
    case Target::amd64:
        amd64::Printer printer { m_frame };
        printer.run(os);
    }
}

FrameMachineAsmWriter::FrameMachineAsmWriter(FrameMachineInfo& frame) 
    : m_frame(frame) {}

void FrameMachineAsmWriter::run(std::ostream& os) {
    switch (m_frame.get_target()->arch()) {
    case Target::amd64:
        amd64::AsmWriter writer { m_frame };
        writer.run(os);
    }
}
