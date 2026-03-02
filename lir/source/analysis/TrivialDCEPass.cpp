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
        for (Instruction* inst = curr->get_head(); inst; inst = inst->get_next()) {
            if (inst->is_trivially_dead())
                m_to_remove.push_back(inst);
        }

        curr = curr->get_next();
    }

    for (Instruction* inst : m_to_remove)
        inst->detach();
}
