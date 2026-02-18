//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/MachineLabel.hpp"
#include "lir/machine/MachineOp.hpp"

using namespace lir;

MachineOp::MachineOp(uint32_t op, const Operands &operands, 
                     MachineLabel *parent)
  : m_op(op), m_pos(0), m_operands(operands), m_parent(parent) {
    if (parent)
        parent->append(this);
}

void MachineOp::detach() {
    assert(has_parent() && "op does not belong to a label!");
    get_parent()->remove(this);
}

const MachineFunction *MachineOp::get_function() const {
    assert(has_parent() && "op does not belong to a label!");
    return get_parent()->get_parent();
}

void MachineOp::insertAfter(MachineOp* op) {
    assert(op && "op cannot be null!");

    op->get_parent()->insertAfter(this, op);
}
