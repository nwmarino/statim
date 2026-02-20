//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/CallsiteAnalysis.h"
#include "lir/machine/LinearScan.h"
#include "lir/machine/RegisterAnalysis.h"
#include "lir/machine/Register.h"
#include "lir/machine/RegisterAllocator.h"

#include <unordered_map>
#include <vector>

using namespace lir;

RegisterAnalysis::RegisterAnalysis(MachineObject& obj) : m_obj(obj) {}

void RegisterAnalysis::run() {
    for (const auto& [name, func] : m_obj.get_functions()) {
        std::vector<LiveRange> ranges = {};

        LinearScan LS = { *func, ranges };
        LS.run();

        RegisterAllocator RA = { *func, ranges };
        RA.run();

        CallsiteAnalysis CA = { m_obj.get_machine(), *func, ranges };
        CA.run();

        // Create a mapping between virtual register ids -> physical register ids.
        std::unordered_map<uint32_t, Register> allocations = {};
        for (const LiveRange& range : ranges) {
            const Register reg = range.reg;

            if (reg.is_physical())
                continue;

            allocations.emplace(reg.id(), range.alloc);
        }

        for (uint32_t i = 0; i < func->num_labels(); ++i) {
            MachineLabel* curr = func->get_label(i);

            MachineOp* op = curr->get_head();
            while (op) {
                for (MachineOperand& operand : op->operands()) {
                    MachineRegister* reg;
                    
                    if (operand.is_reg()) {
                        reg = &operand.reg();
                    } else if (operand.is_mem()) {
                        reg = &operand.mem().base;
                    } else {
                        continue;
                    }

                    if (reg->reg().is_physical())
                        continue;

                    auto alloc = allocations.find(reg->reg().id());
                    assert(alloc != allocations.end() && "virtual register not allocated!");

                    reg->setReg(alloc->second);
                }

                op = op->get_next();
            }
        }

        func->update_positions();
    }
}
