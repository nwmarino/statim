//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/analysis/TrivialDCEPass.h"
#include "lir/graph/BasicBlock.h"

using namespace lir;

void TrivialDCEPass::run() {
    for (Function* func : m_cfg.get_functions())
        process(func);
}

void TrivialDCEPass::process(Function* func) {
    m_to_remove.clear();

    BasicBlock* curr = func->get_head();
    while (curr) {
        // If the basic block has no predecessors and is not the first block
        // in its parent function, then remove it.
        //
        // @Todo: if a block is used as an incoming edge to a phi and gets
        // removed, bad things happen.
        /*
        if (!curr->has_preds() && !curr->is_entry()) {
            BasicBlock* tmp = curr->get_next();
            
            curr->detach();
            delete curr;

            curr = tmp;
            continue;
        }
        */

        for (Instruction* inst = curr->get_head(); inst; inst = inst->get_next()) {
            if (inst->is_trivially_dead()) {
                m_to_remove.push_back(inst);
            } else if (Brif* op = dynamic_cast<Brif*>(inst)) {
                process(op);
            }
        }

        curr = curr->get_next();
    }

    for (Instruction* inst : m_to_remove) {
        inst->detach();
        delete inst;
    }
}

void TrivialDCEPass::process(Brif* op) {
    lir::Integer* cond = dynamic_cast<lir::Integer*>(op->get_cond());
    if (!cond)
        return;

    assert(op->get_parent());

    m_to_remove.push_back(op);

    BasicBlock* parent = op->get_parent();
    m_builder.set_insert(parent);

    if (cond->get_value() == 0) {
        // Condition is a constant zero, so replace the brif with a jump to the
        // false destination.

        op->get_true_dest()->remove_pred(parent);
        parent->remove_succ(op->get_true_dest());
        
        m_builder.build_jump(op->get_false_dest());
    } else {
        // Condition is a constant non-zero, so replace the brif with a jump to
        // the true destination.
        
        op->get_false_dest()->remove_pred(parent);
        parent->remove_succ(op->get_false_dest());

        m_builder.build_jump(op->get_true_dest());
    }
}
