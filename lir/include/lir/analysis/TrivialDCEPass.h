//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_TRIVIAL_DCE_PASS_H_
#define LIR_TRIVIAL_DCE_PASS_H_

#include "lir/analysis/Pass.h"

namespace lir {

/// Function-based pass to remove trivially dead instructions.
class TrivialDCEPass final : public Pass {
    /// A list of instructions to remove after the current process.
    std::vector<Instruction*> m_to_remove = {};

public:
    TrivialDCEPass(CFG &cfg) : Pass(cfg) {}

    ~TrivialDCEPass() = default;

    TrivialDCEPass(const TrivialDCEPass&) = delete;
    void operator=(const TrivialDCEPass&) = delete;

    TrivialDCEPass(TrivialDCEPass&&) noexcept = delete;
    void operator=(TrivialDCEPass&&) noexcept = delete;

    void run() override;

private:
    void process(Function* func);
};

} // namespace lir

#endif // LIR_TRIVIAL_DCE_PASS_H_
