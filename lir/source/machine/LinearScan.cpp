//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/LinearScan.h"
#include "lir/machine/MachineOperand.h"
#include "lir/machine/Register.h"

using namespace lir;

LinearScan::LinearScan(MachineFunction& func, std::vector<LiveRange>& ranges)
  : m_func(func), m_ranges(ranges), m_pos(0) {}

void LinearScan::run() {
    m_func.update_positions();

    for (uint32_t i = 0; i < m_func.num_labels(); ++i) {
        MachineLabel* curr = m_func.get_label(i);

        for (MachineOp* op = curr->get_head(); op; op = op->get_next())
            process(op);
    }
}

void LinearScan::process(MachineOp* op) {
    for (MachineOperand& operand : op->operands()) {
        if (!operand.is_reg() && !operand.is_mem())
            continue;

        MachineRegister reg;
        RegisterClass cls;

        if (operand.is_reg()) {
            reg = operand.reg();
        } else if (operand.is_mem()) {
            reg = operand.mem().base;
        }

        cls = reg.reg().cls();

        LiveRange& range = updateRange(reg.reg(), cls, op->get_pos());
        if (reg.isExpired()) {
            range.end = op->get_pos();
            range.expired = true;
        }
    }
}

LiveRange& LinearScan::updateRange(Register reg, RegisterClass cls, uint32_t pos) {
    for (LiveRange& range : m_ranges) {
        if (range.expired)
            continue;

        if (range.reg == reg) {
            range.end = pos;
            return range;
        }
    }

    LiveRange range = {};
    range.reg = reg;
    range.alloc = Register::NO_REGISTER;
    range.cls = cls;
    range.start = pos;
    range.end = pos;
    range.expired = false;

    if (reg.is_physical())
        range.alloc = reg;

    m_ranges.push_back(range);
    return m_ranges.back();
}
