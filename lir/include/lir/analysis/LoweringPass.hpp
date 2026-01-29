//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_LOWERING_PASS_H_
#define LOVELACE_IR_LOWERING_PASS_H_

#include "lir/analysis/Pass.hpp"
#include "lir/machine/Segment.hpp"

namespace lir {

/// Global pass that generates shallow machine code for a given machine target.
///
/// This pass specifically handles creation of machine equivelants for 
/// functions and global data, while considering machine-specific constraints.
class LoweringPass final : public Pass {
    Segment &m_seg;

public:
    LoweringPass(CFG &cfg, Segment &seg) : Pass(cfg), m_seg(seg) {}

    void run() override;
};

} // namespace lir

#endif // LOVELACE_IR_LOWERING_PASS_H_
