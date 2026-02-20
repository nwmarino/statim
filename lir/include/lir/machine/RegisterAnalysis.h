//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_REGISTER_ANALYSIS_H_
#define LIR_REGISTER_ANALYSIS_H_

#include "lir/machine/MachineObject.h"

namespace lir {
    
class RegisterAnalysis final {
    MachineObject& m_obj;

public:
    RegisterAnalysis(MachineObject& obj);

    void run();
};

} // namespace lir

#endif // LIR_REGISTER_ANALYSIS_H_
