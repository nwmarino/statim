//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_CONSTANT_FOLDING_H_
#define LIR_CONSTANT_FOLDING_H_

#include "lir/analysis/Pass.h"
#include "lir/graph/Instruction.h"

#include <vector>

namespace lir {

/// Function-based pass to fold instructions with constant operands that can
/// be reduced at compile-time.
class ConstantFolding final : public Pass {
    std::vector<Instruction*> m_to_remove = {};

public:
    ConstantFolding(CFG &cfg) : Pass(cfg) {}

    ~ConstantFolding() = default;

    ConstantFolding(const ConstantFolding&) = delete;
    void operator=(const ConstantFolding&) = delete;

    ConstantFolding(ConstantFolding&&) noexcept = delete;
    void operator=(ConstantFolding&&) noexcept = delete;

    void run() override;

private:
    void process(Function* func);

    void process(Binop* op);

    void process(Unop* op);

    void process(Cmp* op);
};

} // namespace lir

#endif // LIR_CONSTANT_FOLDING_H_
