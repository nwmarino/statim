//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/AMD64Analysis.h"
#include "lir/machine/MachineFunction.h"
#include "lir/machine/MachineObject.h"
#include "lir/machine/MachineOp.h"
#include "lir/machine/MachineOperand.h"

using namespace lir;

AMD64Analysis::AMD64Analysis(const Machine& mach, MachineObject& obj)
  : m_mach(mach), m_obj(obj) {}

void AMD64Analysis::run() {
    for (auto& [name, func] : m_obj.get_functions()) {
        process(func);
    }
}

void AMD64Analysis::process(MachineFunction* func) {
    for (uint32_t i = 0; i < func->num_labels(); ++i) {
        MachineLabel* label = func->get_label(i);

        for (MachineOp* op = label->get_head(); op; op = op->get_next()) {
            if (is_redundant_move(op)) {
                MachineOp* tmp = op;
                op = op->get_prev();
                tmp->detach();
            } else if (op->get_next() && is_redundant_move(op, op->get_next())) {
                MachineOp* tmp = op;
                op = op->get_prev();
                tmp->detach();
            }
        }
    }
}

bool AMD64Analysis::is_basic_move(AMD64_Op op) const {
    return op == AMD64_MOV8 || op == AMD64_MOV16 
        || op == AMD64_MOV32 || op == AMD64_MOV64 
        || op == AMD64_MOVSS || op == AMD64_MOVSD;
}

bool AMD64Analysis::is_redundant_move(MachineOp* op) const {
    if (!is_basic_move(static_cast<AMD64_Op>(op->op())))
        return false;
    
    assert(op->num_explicit_operands() == 2);

    MachineOperand& lop = op->get_operand(0);
    MachineOperand& rop = op->get_operand(1);

    if (!(lop.is_reg() && rop.is_reg()))
        return false;

    return lop.reg().reg() == rop.reg().reg() 
        && lop.reg().subreg() == rop.reg().subreg();
}

bool AMD64Analysis::is_redundant_move(MachineOp* first, MachineOp* second) const {
    // @Todo: implement for ops like:
    //
    // movq %rax, %rcx
    // movq %rcx, %r8
    //
    // becomes
    //
    // movq %rax, %r8
    return false;
}
