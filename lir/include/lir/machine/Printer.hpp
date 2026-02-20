//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_PRINTER_H_
#define LIR_PRINTER_H_

#include "lir/machine/Machine.hpp"
#include "lir/machine/MachineObject.hpp"
#include "lir/machine/MachineOp.hpp"

namespace lir {

class Printer final {
    const Machine &m_mach;
    const MachineObject &m_obj;

public:
    Printer(const MachineObject &obj);

    void run(std::ostream &os);

private:
    void print_register(std::ostream &os, const MachineRegister &reg);
    void print_operand(std::ostream &os, const MachineOperand &operand);
    void print_op(std::ostream &os, const MachineOp &op);
    void print_label(std::ostream &os, const MachineLabel &label);
    void print_function(std::ostream &os, const MachineFunction &func);
    void print_constant(std::ostream &os, const MachineConstant &constant);
    void print_data(std::ostream &os, const MachineData &data);
};

} // namespace lir

#endif // LIR_PRINTER_H_
