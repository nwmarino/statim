//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/analysis/LoweringPass.hpp"
//#include "lir/machine/InstSelector.hpp"

using namespace lir;

void LoweringPass::run() {
    for (const Function *func : m_cfg.get_functions()) {
        // Empty functions should not be lowered, they should either be
        // resolved at link time or with some library.
        if (func->empty())
            continue;

        MachFunction* mach_function = new MachFunction(
            func, m_seg.get_machine());
        
        m_seg.get_functions().emplace(func->get_name(), mach_function);

        const BasicBlock* curr = func->get_head();
        while (curr) {
            new MachLabel(curr, mach_function);
            curr = curr->get_next();
        }

        //InstSelector isel(*mach_function);
        //isel.run();
    }
}
