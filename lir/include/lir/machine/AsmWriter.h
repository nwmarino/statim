//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_ASM_WRITER_H_
#define LIR_ASM_WRITER_H_

#include "lir/machine/AMD64.h"
#include "lir/machine/MachineObject.h"
#include "lir/machine/MachineOp.h"

#include <unordered_map>

namespace lir {

class AsmWriter final {
    const MachineObject& m_obj;
    const MachineFunction* m_func = nullptr;

    std::unordered_map<const MachineFunction*, uint32_t> m_ids = {};

public:
    AsmWriter(const MachineObject& obj);

    AsmWriter(const AsmWriter&) = delete;
    void operator=(const AsmWriter&) = delete;

    AsmWriter(AsmWriter&&) noexcept = delete;
    void operator=(AsmWriter&&) noexcept = delete;

    void run(std::ostream& os);

private:
    void writeOpcode(std::ostream& os, AMD64_Op op);

    void writeRegister(std::ostream& os, AMD64_Register reg, uint8_t subreg);

    void writeOperand(std::ostream& os, const MachineOperand& operand);

    void writeOp(std::ostream& os, const MachineOp& op);

    void writeLabel(std::ostream& os, const MachineLabel& label);

    void writeFunction(std::ostream& os, const MachineFunction& func);

    void writeConstant(std::ostream& os, const MachineConstant& constant);

    void writeData(std::ostream& os, const MachineData& data);
};

} // namespace lir

#endif // LIR_ASM_WRITER_H_
