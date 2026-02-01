//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_LOWERING_PASS_H_
#define LOVELACE_IR_LOWERING_PASS_H_

#include "lir/analysis/Pass.hpp"
#include "lir/machine/MachineObject.hpp"

namespace lir {

/// Global pass that generates machine-dependent code.
class LoweringPass final : public Pass {
    MachineObject &m_obj;

public:
    LoweringPass(CFG &cfg, MachineObject &obj) : Pass(cfg), m_obj(obj) {}

    void run() override;
};

} // namespace lir

#endif // LOVELACE_IR_LOWERING_PASS_H_
