//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/MachineFunction.hpp"
#include "lir/machine/MachineObject.hpp"

#include <algorithm>

using namespace lir;

/// Aligns the given |offset| to the provided |alignment|.
static inline uint32_t align_to(uint32_t offset, uint32_t alignment) {
    return (offset + alignment - 1) & ~(alignment - 1);
}

//>==---------------------------------------------------------------------------
//                          MachineLocal Implementation
//>==---------------------------------------------------------------------------

MachineLocal::MachineLocal(StackFrame *parent, int32_t offset, uint32_t size, 
                           uint32_t align)
  : m_parent(parent), m_offset(offset), m_size(size), m_align(align) {
    if (parent)
        parent->get_locals().push_back(this);
}

const MachineFunction *MachineLocal::get_function() const {
    assert(has_parent() && "local does not belong to a stack frame!");
    return get_parent()->get_parent();
}

//>==---------------------------------------------------------------------------
//                          ConstantPool Implementation
//>==---------------------------------------------------------------------------

ConstantPool::~ConstantPool() {
    for (MachineData *constant : m_constants)
        delete constant;
}

MachineData *ConstantPool::materialize(const MachineData::Data &data) {
    const std::string name = std::to_string(m_constants.size());

    MachineData *MD = new MachineData(
        std::to_string(m_constants.size()),
        data,
        false, // private
        true // read only
    );
    assert(MD);

    m_constants.push_back(MD);
    return MD;
}

//>==---------------------------------------------------------------------------
//                          StackFrame Implementation
//>==---------------------------------------------------------------------------

StackFrame::~StackFrame() {
    for (MachineLocal *local : m_locals)
        delete local;
}

uint32_t StackFrame::size() const {
    if (empty())
        return 0;

    const MachineLocal *back = m_locals.back();
    return align_to(back->get_offset() + back->get_size(), 16);
}

//>==---------------------------------------------------------------------------
//                          MachineFunction Implementation
//>==---------------------------------------------------------------------------

MachineFunction::MachineFunction(MachineObject *parent, const FunctionABI &abi, 
                                 const std::string &name)
  : m_parent(parent), m_abi(abi), m_name(name) {
    if (parent)
        parent->get_functions().emplace(name, this);
}

MachineFunction::~MachineFunction() {
    for (MachineLabel *label : m_labels)
        delete label;
}

void MachineFunction::add(MachineLabel *label) {
    assert(label && "label cannot be null!");
    m_labels.push_back(label);
}

void MachineFunction::remove(MachineLabel *label) {
    assert(label && "label cannot be null!");

    auto it = std::find(m_labels.begin(), m_labels.end(), label);
    if (it != m_labels.end())
        m_labels.erase(it);
}

void MachineFunction::update_positions() {
    uint32_t pos = 1;
    for (MachineLabel *label : m_labels) {
        MachineOp *curr = label->get_head();
        while (curr) {
            curr->set_pos(pos++);
            curr = curr->get_next();
        }
    }
}

uint32_t MachineFunction::get_position(const MachineLabel *label) const {
    assert(label && "label cannot be null!");
    assert(label->get_parent() == this && 
        "label does not belong to this function!");

    uint32_t pos = 0;
    for (const MachineLabel *L : m_labels) {
        if (L == label)
            return pos;

        pos++;
    }

    assert(false && "label belongs to this function, but is not registered!");
}
