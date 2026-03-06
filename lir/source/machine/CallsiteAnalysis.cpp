//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/CallsiteAnalysis.h"
#include "lir/machine/AMD64.h"
#include "lir/machine/MachineOp.h"
#include "lir/machine/Register.h"
#include "lir/machine/RegisterAllocator.h"

#include <vector>

using namespace lir;

CallsiteAnalysis::CallsiteAnalysis(const Machine& mach, MachineFunction& func, 
                                   std::vector<LiveRange>& ranges) 
  : m_mach(mach), m_func(func), m_ranges(ranges) {}

void CallsiteAnalysis::run() {
    // @Todo: optimize by keeping track of callsite pointers in the function/label during lowering.

    for (uint32_t i = 0; i < m_func.num_labels(); ++i) {
        MachineLabel* label = m_func.get_label(i);

        MachineOp* set = nullptr;
        MachineOp* call = nullptr;
        MachineOp* end = nullptr;

        for (MachineOp* op = label->get_head(); op; op = op->get_next()) {
            if (op->is_intrinsic()) {
                Intrinsic opcode = static_cast<Intrinsic>(op->op());
                if (opcode == Intrinsic::Callsite_Set) {
                    set = op;
                } else if (opcode == Intrinsic::Callsite_End) {
                    end = op;
                }
            } else if (op->op() == AMD64_CALL32 || op->op() == AMD64_CALL64) {
                call = op;
            }

            if (set && call && end) {
                std::vector<Register> spill = {};

                for (const LiveRange& range : m_ranges) {
                    if (!range.overlaps(call->get_pos()))
                        continue;

                    const Register& alloc = range.alloc;
                    if (m_mach.isCallerSaved(alloc.id()))
                        spill.push_back(alloc);
                }

                if (!spill.empty()) {
                    MachineOp* reserve = new MachineOp(AMD64_SUB64, {});
                    reserve->add_imm(spill.size() * 8);
                    reserve->add_reg({ RSP, 8 });
                    
                    MachineOp* restore = new MachineOp(AMD64_ADD64, {});
                    restore->add_imm(spill.size() * 8);
                    restore->add_reg({ RSP, 8 });

                    reserve->insertAfter(set);
                    restore->insertBefore(end);
                }

                for (int32_t i = spill.size() - 1; i >= 0; --i) {
                    const Register& reg = spill[i];

                    MachineOp* push = new MachineOp(AMD64_PUSH64, {});
                    push->add_reg({ reg, 8 });
                    push->add_comment("8-byte spill\n");

                    MachineOp* pop = new MachineOp(AMD64_POP64, {});
                    pop->add_reg({ reg, 8 });
                    pop->add_comment("8-byte reload\n");

                    push->insertAfter(set);
                    pop->insertBefore(end);
                }

                set = nullptr;
                call = nullptr;
                end = nullptr;
            }
        }
    }    
}
