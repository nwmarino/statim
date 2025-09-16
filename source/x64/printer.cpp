#include "siir/machine_basicblock.hpp"
#include "siir/machine_function.hpp"
#include "siir/machine_inst.hpp"
#include "siir/machine_operand.hpp"
#include "siir/machine_register.hpp"
#include "x64/x64.hpp"

#include <iomanip>

using namespace stm;
using namespace stm::siir;
using namespace stm::siir::x64;

static const FunctionRegisterInfo* g_register_info = nullptr;

static void print_operand(std::ostream& os, const MachineOperand& MO) {
    switch (MO.get_kind()) {
    case MachineOperand::MO_Register: {
        MachineRegister reg = MO.get_reg();
        if (reg.is_virtual()) {
            os << 'v' << (reg.id() - MachineRegister::VirtualBarrier) << 
                ':' << MO.get_subreg();
        } else {
            os << "%" << x64::to_string(
                static_cast<x64::Register>(reg.id()), MO.get_subreg());
        }

        break;
    }

    case MachineOperand::MO_Memory: {
        os << "[";

        MachineRegister reg = MO.get_mem_base();
        if (reg.is_virtual()) {
            os << 'v' << (reg.id() - MachineRegister::VirtualBarrier);
        } else {
            os << "%" << x64::to_string(
                static_cast<x64::Register>(reg.id()), 64);
        }

        if (MO.get_mem_disp() > 0)
            os << '+';

        os << MO.get_mem_disp() << ']';
        break;
    }

    case MachineOperand::MO_Immediate:
        os << '$' << MO.get_imm();
        break;

    case MachineOperand::MO_BasicBlock:
        os << ".LBB" << MO.get_mmb()->position();
        break;
    
    case MachineOperand::MO_Symbol: 
        os << MO.get_symbol();
        break;
    }
}

static void print_inst(std::ostream& os, const MachineInst& MI) {
    os << std::setw(10) << x64::to_string(
        static_cast<x64::Opcode>(MI.opcode())) << "    ";

    for (u32 idx = 0, e = MI.num_operands(); idx != e; ++idx) {
        print_operand(os, MI.get_operand(idx));
        if (idx + 1 != e)
            os << ", ";
    }

    if (MI.num_implicit_operands() > 0) {
        os << "    ... ";
        for (u32 idx = MI.num_explicit_operands(), e = MI.num_operands(); idx != e; ++idx) {
            print_operand(os, MI.get_operand(idx));
            if (idx + 1 != e)
                os << ", ";
        }
    }
}

static void print_block(std::ostream& os, const MachineBasicBlock& MBB) {
    os << ".LBB" << MBB.position() << ":\n";

    for (auto inst : MBB.insts()) {
        print_inst(os, inst);
        os << "\n";
    }
}

static void print_function(std::ostream& os, const MachineFunction& MF) {
    g_register_info = &MF.get_register_info();

    os << MF.get_name() << ":\n";

    for (auto curr = MF.front(); curr; curr = curr->next())
        print_block(os, *curr);
}

void x64::X64Printer::run(std::ostream& os) const {
    g_register_info = nullptr;

    //for (auto function : m_obj.functions()) {
    //    print_function(os, *function);
    //    os << "\n";
    //}
}
