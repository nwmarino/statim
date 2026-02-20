//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_PASS_H_
#define LOVELACE_IR_PASS_H_

#include "lir/graph/CFG.hpp"

namespace lir {

class Pass {
protected:
    CFG &m_cfg;

public:
    Pass(CFG &cfg) : m_cfg(cfg) {}

    virtual ~Pass() = default;

    Pass(const Pass&) = delete;
    void operator=(const Pass&) = delete;
    
    Pass(Pass&&) noexcept = delete;
    void operator=(Pass&&) noexcept = delete;

    virtual void run() = 0;
};

} // namespace lir

#endif // LOVELACE_IR_PASS_H_
