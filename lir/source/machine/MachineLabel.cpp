//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/MachineFunction.h"
#include "lir/machine/MachineLabel.h"

using namespace lir;

MachineLabel::MachineLabel(MachineFunction *parent) : m_parent(parent) {
    if (parent)
        parent->add(this);
}

void MachineLabel::prepend(MachineOp *op) {
    assert(op && "op cannot be null!");

    if (m_head) {
        op->set_next(m_head);
        m_head->set_prev(op);

        m_head = op;
    } else {
        m_head = op, m_tail = op;
    }

    op->set_parent(this);
}

void MachineLabel::append(MachineOp *op) {
    assert(op && "op cannot be null!");

    if (m_tail) {
        op->set_prev(m_tail);
        m_tail->set_next(op);

        m_tail = op;
    } else {
        m_head = op, m_tail = op;
    }

    op->set_parent(this);
}

void MachineLabel::insertBefore(MachineOp* op, MachineOp* target) {
    assert(op && "op cannot be null!");
    assert(!op->has_parent() && "op already belongs to a label!");
    assert(target && "target cannot be null!");
    assert(target->get_parent() == this && "target does not belong to this label!");

    if (m_head == target)
        m_head = op;

    if (target->get_prev())
        target->get_prev()->set_next(op);

    op->set_next(target);
    op->set_prev(target->get_prev());
    op->set_parent(this);

    target->set_prev(op);
}

void MachineLabel::insertAfter(MachineOp* op, MachineOp* target) {
    assert(op && "op cannot be null!");
    assert(!op->has_parent() && "op already belongs to a label!");
    assert(target && "target cannot be null!");
    assert(target->get_parent() == this && "target does not belong to this label!");

    if (m_tail == target)
        m_tail = op;

    if (target->get_next())
        target->get_next()->set_prev(op);

    op->set_prev(target);
    op->set_next(target->get_next());
    op->set_parent(this);

    target->set_next(op);
}

void MachineLabel::remove(MachineOp *op) {
    assert(op && "op cannot be null!");
    assert(op->get_parent() == this && "op does not belong to this label!");

    if (op->get_prev()) {
        op->get_prev()->set_next(op->get_next());
    } else {
        if (op->get_next()) {
            m_head = op->get_next();
        } else {
            m_head = nullptr, m_tail = nullptr;
        }
    }

    if (op->get_next()) {
        op->get_next()->set_prev(op->get_prev());
    } else {
        if (op->get_prev()) {
            m_tail = op->get_prev();
        } else {
            m_head = nullptr, m_tail = nullptr;
        }
    }

    op->set_prev(nullptr);
    op->set_next(nullptr);
    op->set_parent(nullptr);
}

uint32_t MachineLabel::position() const {
    assert(has_parent() && "label does not belong to a function!");
    return get_parent()->get_position(this);
}
