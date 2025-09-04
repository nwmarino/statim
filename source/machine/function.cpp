#include "bytecode.hpp"
#include "machine/function.hpp"

using namespace stm;

MachineFunction::MachineFunction(
        const Function* fn, 
        const Target& target)
    : m_fn(fn), m_target(target) {}

MachineFunction::~MachineFunction() {
    /// TODO: Clean this up to reset links.
    for (auto curr = m_front; curr; curr = curr->next())
        delete curr;

    m_front = nullptr;
    m_back = nullptr;
}

const std::string& MachineFunction::get_name() const {
    assert(m_fn);
    return m_fn->get_name();
}

const MachineBasicBlock* MachineFunction::at(u32 idx) const {
    u32 pos = 0;
    for (auto curr = m_front; curr; curr = curr->next())
        if (pos++ == idx) return curr;
    return nullptr;
}

MachineBasicBlock* MachineFunction::at(u32 idx) {
    u32 pos = 0;
    for (auto curr = m_front; curr; curr = curr->next())
        if (pos++ == idx) return curr;

    return nullptr;
}

u32 MachineFunction::size() const {
    u32 size = 0;
    for (auto curr = m_front; curr; curr = curr->next()) size++;
    return size;
}

void MachineFunction::prepend(MachineBasicBlock* mbb) {
    assert(mbb);

    if (m_front) {
        m_front->set_prev(mbb);
        mbb->set_next(m_front);
        m_front = mbb;
    } else {    
        m_front = mbb;
        m_back = mbb;
    }
}

void MachineFunction::append(MachineBasicBlock* mbb) {
    assert(mbb);
    
    if (m_back) {
        m_back->set_next(mbb);
        mbb->set_prev(m_back);
        m_back = mbb;
    } else {
        m_front = mbb;
        m_back = mbb;
    }
}
