#include "siir/machine_basicblock.hpp"
#include "siir/machine_function.hpp"
#include "siir/machine_inst.hpp"
#include "siir/machine_operand.hpp"
#include "siir/machine_register.hpp"
#include "x64/x64.hpp"

using namespace stm;
using namespace stm::siir;
using namespace stm::siir::x64;

static const FunctionRegisterInfo* g_register_info = nullptr;

static void print_operand(std::ostream& os, const MachineFunction& MF, 
                          const MachineOperand& MO) {
    switch (MO.kind()) {
    case MachineOperand::MO_Register: {
        if (MO.is_def()) {
            if (MO.is_implicit())
                os << "implicit-def ";

            if (MO.is_dead())
                os << "dead ";
        } else if (MO.is_use()) {
            if (MO.is_implicit())
                os << "implicit ";

            if (MO.is_kill())
                os << "killed ";
        }

        const FunctionRegisterInfo& regi = MF.get_register_info();
        MachineRegister reg = MO.get_reg();
        if (reg.is_virtual()) {
            MachineRegister alloc = regi.vregs.at(reg.id()).alloc;
            if (alloc != MachineRegister::NoRegister)
                reg = alloc;
        }

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
        os << '[';

        MachineRegister reg = MO.get_mem_base();
        if (reg.is_virtual()) {
            os << 'v' << (reg.id() - MachineRegister::VirtualBarrier);
        } else {
            os << '%' << x64::to_string(
                static_cast<x64::Register>(reg.id()), 64);
        }

        if (MO.get_mem_disp() != 0) {
            if (MO.get_mem_disp() > 0)
                os << '+';

            os << MO.get_mem_disp();
        }

        os << ']';
        break;
    }

    case MachineOperand::MO_StackIdx:
        os << "stack." << MO.get_stack_index();
        break;

    case MachineOperand::MO_Immediate:
        os << '$' << MO.get_imm();
        break;

    case MachineOperand::MO_BasicBlock:
        os << "bb" << MO.get_mmb()->position();
        break;

    case MachineOperand::MO_ConstantIdx:
        os << "const." << MO.get_constant_index();
        break;
    
    case MachineOperand::MO_Symbol: 
        os << MO.get_symbol();
        break;
    }
}

static void print_inst(std::ostream& os, const MachineFunction& MF,
                      const MachineInst& MI) {
    os << "    ";

    if (MI.num_explicit_defs() == 1) {
        for (auto& MO : MI.operands()) {
            if (MO.is_reg() && MO.is_explicit_def()) {
                print_operand(os, MF, MO);
                break;
            }
        }

        os << " = ";
    }

    os << x64::to_string(static_cast<x64::Opcode>(MI.opcode())) << " ";

    for (u32 idx = 0, e = MI.num_operands(); idx != e; ++idx) {
        const MachineOperand& mo = MI.get_operand(idx);
        if (MI.num_explicit_defs() == 1) {
            if (mo.is_reg() && mo.is_explicit_def())
                continue;
        }

        print_operand(os, MF, mo);
        if (idx + 1 != e) {
            const MachineOperand& next = MI.get_operand(idx + 1);
            if (!next.is_reg() || !next.is_explicit_def())
                os << ", ";
        }
    }
}

static void print_block(std::ostream& os, const MachineFunction& MF,
                        const MachineBasicBlock& MBB) {
    os << "bb" << MBB.position() << ":\n";

    for (auto inst : MBB.insts()) {
        print_inst(os, MF, inst);
        os << '\n';
    }
}

static void print_function(std::ostream& os, const MachineFunction& MF) {
    g_register_info = &MF.get_register_info();

    os << MF.get_name() << ":\n";

    const FunctionStackInfo& stack = MF.get_stack_info();
    for (u32 idx = 0, e = stack.num_entries(); idx != e; ++idx) {
        const FunctionStackEntry& entry = stack.entries[idx];
        os << "    stack." << idx << " offset: " << entry.offset << ", size: " << 
            entry.size << ", align: " << entry.align << '\n';
    }

    const FunctionConstantPool& pool = MF.get_constant_pool();
    for (u32 idx = 0, e = pool.num_entries(); idx != e; ++idx) {
        const FunctionConstantPoolEntry& entry = pool.entries[idx];
        os << "    const." << idx << ' ' << 
            entry.constant->get_type()->to_string() << ' ';
        entry.constant->print(os);
        os << '\n';
    }

    if (stack.num_entries() > 0 || pool.num_entries() > 0 )
        os << '\n';

    for (auto curr = MF.front(); curr; curr = curr->next())
        print_block(os, MF, *curr);
}

void x64::X64Printer::run(std::ostream& os) const {
    g_register_info = nullptr;

    for (const auto& [name, function] : m_obj.functions()) {
        print_function(os, *function);
        os << '\n';
    }
}
