#ifndef STATIM_SIIR_MACHINE_ANALYSIS_H_
#define STATIM_SIIR_MACHINE_ANALYSIS_H_

#include "siir/cfg.hpp"
#include "siir/machine_object.hpp"

namespace stm::siir {

/// Analysis pass to lower an SIIR graph to a target-dependent representation.
class CFGMachineAnalysis final {
    CFG& m_cfg;

public:
    CFGMachineAnalysis(CFG& cfg);

    CFGMachineAnalysis(const CFGMachineAnalysis&) = delete;
    CFGMachineAnalysis& operator = (const CFGMachineAnalysis&) = delete;

    void run(MachineObject& obj);
};

/// Machine analysis pass to do liveness analysis, register allocation, etc.
class FunctionRegisterAnalysis final {
    MachineObject& m_obj;

public:
    FunctionRegisterAnalysis(MachineObject& obj);
    
    FunctionRegisterAnalysis(const FunctionRegisterAnalysis&) = delete;
    FunctionRegisterAnalysis& operator = (const FunctionRegisterAnalysis&) = delete;

    void run();
};

/// Machine pass to dump assembly with optimizer details.
class MachineObjectPrinter final {
    MachineObject& m_obj;

public:
    MachineObjectPrinter(MachineObject& obj);
    
    MachineObjectPrinter(const MachineObjectPrinter&) = delete;
    MachineObjectPrinter& operator = (const MachineObjectPrinter&) = delete;

    void run(std::ostream& os);
};

/// Machine pass to emit final assembly code.
class MachineObjectAsmWriter final {
    MachineObject& m_obj;

public:
    MachineObjectAsmWriter(MachineObject& obj);
    
    MachineObjectAsmWriter(const MachineObjectAsmWriter&) = delete;
    MachineObjectAsmWriter& operator = (const MachineObjectAsmWriter&) = delete;

    void run(std::ostream& os);
};

} // namespace stm::siir

#endif // STATIM_SIIR_MACHINE_ANALYSIS_H_
