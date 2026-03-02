//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_AMD64_ANALYSIS_H_
#define LIR_AMD64_ANALYSIS_H_

#include "lir/machine/AMD64.h"
#include "lir/machine/Machine.h"
#include "lir/machine/MachineFunction.h"

namespace lir {

class AMD64Analysis final {
    const Machine& m_mach;
    MachineObject& m_obj;

public:
    AMD64Analysis(const Machine& mach, MachineObject& obj);

    ~AMD64Analysis() = default;

    AMD64Analysis(const AMD64Analysis&) = delete;
    void operator=(const AMD64Analysis&) = delete;

    AMD64Analysis(AMD64Analysis&&) noexcept = delete;
    void operator=(AMD64Analysis&&) noexcept = delete;

    void run();

private:
    void process(MachineFunction* func);

    bool is_basic_move(AMD64_Op op) const;

    bool is_redundant_move(MachineOp* op) const;

    bool is_redundant_move(MachineOp* first, MachineOp* second) const;
};

} // namespace lir

#endif // LIR_AMD64_ANALYSIS_H_
