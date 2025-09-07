#include "machine/basicblock.hpp"
#include "machine/function.hpp"

using namespace stm;

MachineBasicBlock::MachineBasicBlock(
        const BasicBlock* bb, 
        MachineFunction* parent)
    : m_bb(bb), m_parent(parent) {
    if (m_parent)
        m_parent->append(this);
}

u32 MachineBasicBlock::position() const {
    u32 num = 0;
    const MachineBasicBlock* prev = m_prev;
    while (prev) {
        prev = prev->prev();
        num++;
    }

    return num;
}
