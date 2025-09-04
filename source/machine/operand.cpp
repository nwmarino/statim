#include "machine/operand.hpp"
#include "machine/register.hpp"
#include "target/amd64.hpp"

#include <cassert>

using namespace stm;

MachineOperand MachineOperand::create_reg(
        MachineRegister reg, 
        u16 subreg,
        bool is_def, 
        bool is_implicit, 
        bool is_kill, 
        bool is_dead) {
    assert(!(is_dead && !is_def));
    assert(!(is_kill && is_def));
    MachineOperand operand;
    operand.m_kind = MO_Register;
    operand.m_reg = reg;
    operand.m_subreg = subreg;
    operand.m_is_def = is_def;
    operand.m_is_implicit = is_implicit;
    operand.is_kill_or_dead = is_kill | is_dead;
    return operand;
}

MachineOperand MachineOperand::create_mem(MachineRegister reg, i32 disp) {
    MachineOperand operand;
    operand.m_kind = MO_Memory;
    operand.m_mem.reg = reg;
    operand.m_mem.disp = disp;
    return operand;
}

MachineOperand MachineOperand::create_imm(i64 imm) {
    MachineOperand operand;
    operand.m_kind = MO_Immediate;
    operand.m_imm = imm;
    return operand;
}

MachineOperand MachineOperand::create_block(MachineBasicBlock* mbb) {
    MachineOperand operand;
    operand.m_kind = MO_BasicBlock;
    operand.m_mbb = mbb;
    return operand;
}

MachineOperand MachineOperand::create_symbol(const char* symbol) {
    MachineOperand operand;
    operand.m_kind = MO_Symbol;
    operand.m_symbol = symbol;
    return operand;
}
