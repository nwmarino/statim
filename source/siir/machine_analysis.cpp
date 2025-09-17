#include "siir/cfg.hpp"
#include "siir/function.hpp"
#include "siir/machine_analysis.hpp"
#include "siir/machine_basicblock.hpp"
#include "siir/machine_function.hpp"
#include "siir/machine_object.hpp"
#include "x64/x64.hpp"

using namespace stm;
using namespace stm::siir;

CFGMachineAnalysis::CFGMachineAnalysis(CFG& cfg) : m_cfg(cfg) {}

void CFGMachineAnalysis::run(MachineObject& obj) {
    for (const auto& function : m_cfg.functions()) {
        // Empty functions should not be lowered, they should either be
        // resolved at link time or with some library.
        if (function->empty())
            continue;

        MachineFunction* mf = new MachineFunction(function, *obj.get_target());
        obj.functions().emplace(mf->get_name(), mf);

        for (auto curr = function->front(); curr; curr = curr->next())
            new MachineBasicBlock(curr, mf);

        switch (obj.get_target()->arch()) {
        case Target::x64: {
            x64::X64InstSelection isel { mf };
            isel.run();
            break;
        }

        default:
            assert(false && "unsupported architecture!");
        }
    }
}

FunctionRegisterAnalysis::FunctionRegisterAnalysis(MachineObject& obj)
    : m_obj(obj) {}

void FunctionRegisterAnalysis::run() {
    for (const auto& [name, function] : m_obj.functions()) {
        /// TODO: Implement after register allocation, callsite info, etc.
    }
}

MachineObjectPrinter::MachineObjectPrinter(MachineObject& obj) : m_obj(obj) {}

void MachineObjectPrinter::run(std::ostream& os) {
    switch (m_obj.get_target()->arch()) {
    case Target::x64: {
        x64::X64Printer printer { m_obj };
        printer.run(os);
        break;
    }

    default:
        assert(false && "unsupported architecture!");
    }
}

MachineObjectAsmWriter::MachineObjectAsmWriter(MachineObject& obj) 
    : m_obj(obj) {}

void MachineObjectAsmWriter::run(std::ostream& os) {
    switch (m_obj.get_target()->arch()) {
    case Target::x64: {
        x64::X64AsmWriter writer { m_obj };
        writer.run(os);
        break;
    }

    default:
        assert(false && "unsupported architecture!");
    }
}
