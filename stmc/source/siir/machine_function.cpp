#include "siir/function.hpp"
#include "siir/machine_function.hpp"

using namespace stm;
using namespace stm::siir;

MachineFunction::MachineFunction(const Function* fn, const Target& target)
    : m_fn(fn), m_target(target) {}

MachineFunction::~MachineFunction() {
    auto curr = m_front;
    while (curr) {
        auto tmp = curr->next();
        curr->m_prev = nullptr;
        curr->m_next = nullptr;
        delete curr;

        curr = tmp;
    }

    m_front = nullptr;
    m_back = nullptr;
}

const std::string& MachineFunction::get_name() const {
    assert(m_fn && "machine function does not have an SIIR function!");
    return m_fn->get_name();
}

const MachineBasicBlock* MachineFunction::at(u32 idx) const {
    u32 pos = 0;
    for (auto curr = m_front; curr; curr = curr->next())
        if (pos++ == idx) 
            return curr;

    return nullptr;
}

MachineBasicBlock* MachineFunction::at(u32 idx) {
    u32 pos = 0;
    for (auto curr = m_front; curr; curr = curr->next())
        if (pos++ == idx) 
            return curr;

    return nullptr;
}

u32 MachineFunction::size() const {
    u32 size = 0;
    for (auto curr = m_front; curr; curr = curr->next()) 
        size++;

    return size;
}

void MachineFunction::prepend(MachineBasicBlock* mbb) {
    assert(mbb && "basic block cannot be null!");

    if (m_front) {
        m_front->set_prev(mbb);
        mbb->set_next(m_front);
        m_front = mbb;
    } else {    
        m_front = mbb;
        m_back = mbb;
    }

    mbb->set_parent(this);
}

void MachineFunction::append(MachineBasicBlock* mbb) {
    assert(mbb && "basic block cannot be null!");
    
    if (m_back) {
        m_back->set_next(mbb);
        mbb->set_prev(m_back);
        m_back = mbb;
    } else {
        m_front = mbb;
        m_back = mbb;
    }

    mbb->set_parent(this);
}
