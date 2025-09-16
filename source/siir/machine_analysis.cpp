#include "siir/cfg.hpp"
#include "siir/function.hpp"
#include "siir/machine_analysis.hpp"
#include "siir/machine_basicblock.hpp"
#include "siir/machine_function.hpp"
#include "siir/machine_object.hpp"
#include "x64/x64.hpp"

using namespace stm;
using namespace stm::siir;

CFGMachineAnalysis::CFGMachineAnalysis(CFG& cfg, Target* target)
    : m_cfg(cfg), m_target(target) {}

void CFGMachineAnalysis::run(MachineObject& obj) {
    for (auto function : m_cfg.functions()) {
        MachineFunction* mf = new MachineFunction(function, *m_target);
        // obj.functions += [mf.name, mf] 
        
        for (auto curr = function->front(); curr; curr = curr->next())
            new MachineBasicBlock(curr, mf);

        switch (m_target->arch()) {
        case Target::x64:
            x64::X64InstSelection isel { mf };
            isel.run();
        }
    }
}

FunctionRegisterAnalysis::FunctionRegisterAnalysis(MachineObject& obj)
    : m_obj(obj) {}

void FunctionRegisterAnalysis::run() {
    //for (auto function : m_obj.functions()) {
    //
    //}
}

MachineObjectPrinter::MachineObjectPrinter(MachineObject& obj)
    : m_obj(obj) {}

void MachineObjectPrinter::run(std::ostream& os) {
    switch (m_obj.get_target()->arch()) {
    case Target::x64:
        x64::X64Printer printer { m_obj };
        printer.run(os);
    }
}

MachineObjectAsmWriter::MachineObjectAsmWriter(MachineObject& obj) 
    : m_obj(obj) {}

void MachineObjectAsmWriter::run(std::ostream& os) {
    switch (m_obj.get_target()->arch()) {
    case Target::x64:
        x64::X64AsmWriter writer { m_obj };
        writer.run(os);
    }
}
