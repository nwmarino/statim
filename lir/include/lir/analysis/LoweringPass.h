//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_LOWERING_PASS_H_
#define LOVELACE_IR_LOWERING_PASS_H_

#include "lir/analysis/Pass.h"
#include "lir/machine/MachineObject.h"

namespace lir {

/// Global pass that generates machine-dependent code.
class LoweringPass : public Pass {
protected:
    const Machine &m_mach;
    MachineObject &m_obj;

public:
    LoweringPass(CFG &cfg, MachineObject &obj);

    virtual ~LoweringPass() = default;

    LoweringPass(const LoweringPass&) = delete;
    void operator=(const LoweringPass&) = delete;

    LoweringPass(LoweringPass&&) noexcept = delete;
    void operator=(LoweringPass&&) noexcept = delete;

protected:
    /// Lower the given constant |C| into another value entry for |data|.
    void lower_constant(const Constant *C, std::vector<MachineConstant>& data) const;
};

} // namespace lir

#endif // LOVELACE_IR_LOWERING_PASS_H_
