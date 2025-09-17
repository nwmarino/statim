#include "siir/machine_basicblock.hpp"
#include "siir/machine_inst.hpp"
#include "siir/machine_operand.hpp"

using namespace stm;
using namespace stm::siir;

MachineInst::MachineInst(u32 opcode, 
                         const std::vector<MachineOperand>& operands, 
                         MachineBasicBlock* parent)
        : m_opcode(opcode), m_operands(operands), m_parent(parent) {

    if (m_parent)
        m_parent->push_back(*this);
}

const MachineFunction* MachineInst::get_mf() const {
    if (!m_parent)
        return nullptr;

    return m_parent->get_parent();
}

MachineFunction* MachineInst::get_mf() {
    if (!m_parent)
        return nullptr;

    return m_parent->get_parent();
}

u32 MachineInst::num_defs() const {
    u32 num = 0;
    for (auto mo : m_operands)
        if (mo.is_reg() && mo.is_def())
            num++;

    return num;
}

u32 MachineInst::num_implicit_operands() const {
    u32 num = 0;
    for (auto mo : m_operands)
        if (mo.is_reg() && mo.is_implicit())
            num++;

    return num;
}

u32 MachineInst::num_explicit_operands() const {
    u32 num = 0;
    for (auto mo : m_operands) {
        if (mo.is_reg()) {
            if (!mo.is_implicit())
                ++num;
        } else {
            ++num;
        }
    }

    return num;
}

u32 MachineInst::num_implicit_defs() const {
    u32 num = 0;
    for (auto mo : m_operands)
        if (mo.is_reg() && mo.is_def() && mo.is_implicit())
            num++;

    return num;
}

u32 MachineInst::num_explicit_defs() const {
    u32 num = 0;
    for (auto mo : m_operands)
        if (mo.is_reg() && mo.is_def() && !mo.is_implicit())
            num++;

    return num;
}

bool MachineInst::has_implicit_def() const {
    for (auto mo : m_operands)
        if (mo.is_reg() && mo.is_implicit() && mo.is_def())
            return true;

    return false;
}
