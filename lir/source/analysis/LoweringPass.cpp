//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/analysis/LoweringPass.hpp"
#include "lir/machine/MachineFunction.hpp"
#include "lir/machine/MachineOp.hpp"

using namespace lir;

void LoweringPass::run() {
    for (const Function *func : m_cfg.get_functions()) {
        // Empty functions should not be lowered, they should either be
        // resolved at link time or with some library.
        if (func->empty())
            continue;

        MachineFunction *MF = new MachineFunction(&m_obj, func->get_name());
        assert(MF);

        const BasicBlock *curr = func->get_head();
        while (curr) {
            MachineLabel *ML = new MachineLabel(MF);
            assert(ML);
            
            curr = curr->get_next();
        }
    }
}
