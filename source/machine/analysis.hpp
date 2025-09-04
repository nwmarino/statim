#ifndef STATIM_MACHINE_ANALYSIS_H_
#define STATIM_MACHINE_ANALYSIS_H_

#include "machine/object.hpp"
#include "siir/cfg.hpp"
#include "siir/target.hpp"

namespace stm {

/// Analysis pass to lower bytecode to a target-dependent representation.
class CFGMachineAnalysis final {
    siir::CFG& m_cfg;
    siir::Target* m_target;

public:
    CFGMachineAnalysis(siir::CFG& cfg, siir::Target* target);

    CFGMachineAnalysis(const CFGMachineAnalysis&) = delete;
    CFGMachineAnalysis& operator = (const CFGMachineAnalysis&) = delete;

    void run();
};

/// Analysis pass to do liveness analysis, register allocation, etc.
class FunctionRegisterAnalysis final {
    MachineObject& m_obj;

public:
    FunctionRegisterAnalysis(MachineObject& frame);
    
    FunctionRegisterAnalysis(const FunctionRegisterAnalysis&) = delete;
    FunctionRegisterAnalysis& operator = (const FunctionRegisterAnalysis&) = delete;

    void run();
};

/// Frame pass to dump machine code with optimizer details.
class MachineObjectPrinter final {
    MachineObject& m_obj;

public:
    MachineObjectPrinter(MachineObject& frame);
    
    MachineObjectPrinter(const MachineObjectPrinter&) = delete;
    MachineObjectPrinter& operator = (const MachineObjectPrinter&) = delete;

    void run(std::ostream& os);
};

/// Frame pass to emit final machine code.
class MachineObjectAsmWriter final {
    MachineObject& m_obj;

public:
    MachineObjectAsmWriter(MachineObject& frame);
    
    MachineObjectAsmWriter(const MachineObjectAsmWriter&) = delete;
    MachineObjectAsmWriter& operator = (const MachineObjectAsmWriter&) = delete;

    void run(std::ostream& os);
};;

} // namespace stm

#endif // STATIM_MACHINE_ANALYSIS_H_
