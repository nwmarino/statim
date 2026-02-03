//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/AMD64.hpp"
#include "lir/machine/MachineFunction.hpp"
#include "lir/machine/MachineLabel.hpp"
#include "lir/machine/MachineOp.hpp"
#include "lir/machine/MachineOperand.hpp"
#include "lir/machine/Printer.hpp"

#include <format>
#include <iomanip>
#include <ostream>

using namespace lir;

Printer::Printer(const MachineObject &obj) 
  : m_obj(obj), m_mach(obj.get_machine()) {}

void Printer::run(std::ostream &os) {
    for (const auto &[name, global] : m_obj.get_globals()) {
        print_data(os, *global);
        os << '\n';
    }
    
    for (const auto &[name, func] : m_obj.get_functions()) {
        print_function(os, *func);
        os << '\n';
    }
}

void Printer::print_register(std::ostream &os, const MachineRegister &reg) {
    if (reg.get_register().is_virtual()) {
        os << "%v" << reg.get_register().id() - Register::VIRTUAL_BARRIER;
    } else {
        // @Todo: Change cast for other archs.
        os << '%' << to_string(static_cast<AMD64_Register>(reg.get_register().id()), 0);
    }

    os << std::format(":{}", reg.get_subreg());
}

void Printer::print_operand(std::ostream &os, const MachineOperand &operand) {
    switch (operand.kind()) {
        case MachineOperand::Kind::Register:
            print_register(os, operand.reg());
            break;

        case MachineOperand::Kind::Memory: {
            const Memory &mem = operand.mem();
        
            os << std::format("<mem> ");
            print_register(os, mem.base);

            if (mem.offset >= 0)
                os << '+';
            
            os << mem.offset;
            break;
        }

        case MachineOperand::Kind::Immediate:
            os << std::format("${}", operand.imm());
            break;

        case MachineOperand::Kind::Data:
            os << std::format("<data {}>", operand.data()->get_name());
            break;

        case MachineOperand::Kind::Local:
            os << std::format("<stack {}>", operand.local()->get_offset());
            break;

        case MachineOperand::Kind::Function:
            os << operand.function()->get_name();
            break;

        case MachineOperand::Kind::Label:
            os << std::format(".{}", operand.label()->position());
            break;
    }
}

void Printer::print_op(std::ostream &os, const MachineOp &op) {
    if (op.has_comment())
        os << std::format("\t> {}", op.get_comment());

    os << '\t' << std::setfill('0') << std::setw(5) << op.get_pos() << ' ';

    std::string opstr;
    if (op.is_intrinsic()) switch (static_cast<Intrinsic>(op.op())) {
        case Intrinsic::Husk:
            opstr = "HUSK";
            break;
        case Intrinsic::Param:
            opstr = "PARAM";
            break;
        case Intrinsic::Stack_Setup:
            opstr = "STACK_SETUP";
            break;
        case Intrinsic::Stack_Reserve:
            opstr = "STACK_RESERVE";
            break;
        case Intrinsic::Stack_Restore:
            opstr = "STACK_RESTORE";
            break;
        case Intrinsic::Callsite_Set:
            opstr = "CALLSITE_SET";
            break;
        case Intrinsic::Callsite_End:
            opstr = "CALLSITE_END";
            break;
    } else {
        opstr = to_string(static_cast<AMD64_Op>(op.op()));
    }

    if (!op.has_operands()) {
        os << opstr << '\n';
        return;
    }

    os << opstr << std::string(18 - opstr.size(), ' ');

    for (uint32_t i = 0, e = op.num_operands(); i < e; ++i) {
        print_operand(os, op.get_operand(i));
        if (i + 1 != e)
            os << ", ";
    }

    os << '\n';
}

void Printer::print_label(std::ostream &os, const MachineLabel &label) {
    os << std::format(".{}:\n", label.position());

    const MachineOp *curr = label.get_head();
    while (curr) {
        print_op(os, *curr);
        curr = curr->get_next();
    }
}

void Printer::print_function(std::ostream &os, const MachineFunction &func) {
    os << std::format("{}:\n", func.get_name());

    const FunctionABI &abi = func.abi();
    if (abi.has_result() || abi.num_params() > 0) {
        // Assume stack ABI.
        os << "<abi>\n";

        if (abi.has_result()) {
            os << std::format("\t:{} result\n", 
                abi.get_result_location().offset);
        }

        for (uint32_t i = 0; i < abi.num_params(); ++i) {
            os << std::format("\t:{} param {}\n", 
                abi.get_param_location(i).offset, i + 1);
        }
    }

    const ConstantPool &pool = func.get_pool();
    if (!pool.empty()) {
        os << "<pool>\n";

        for (const MachineData *constant : pool.get_constants()) {
            os << std::format(".C{}:\n", constant->get_name());
        }
    }

    const StackFrame &frame = func.get_stack_frame();
    if (!frame.empty()) {
        os << "<stack>\n";

        for (const MachineLocal *local : frame.get_locals()) {
            os << std::format("\t:{} size {} [{}]\n", 
                local->get_offset(), local->get_size(), local->get_align());
        }
    }

    for (const MachineLabel *label : func.labels()) {
        print_label(os, *label);
    }
}

void Printer::print_constant(std::ostream &os, const MachineConstant &constant) {
    switch (constant.kind()) {
        case MachineConstant::Kind::Zero:
            os << std::format("<zero> {}", constant.get_zeros());
            break;
        case MachineConstant::Kind::Int8:
            os << std::format("<int8> {:#02X} ({})", constant.get_int(), constant.get_int());
            break;
        case MachineConstant::Kind::Int16:
            os << std::format("<int8> {:#04X} ({})", constant.get_int(), constant.get_int());
            break;
        case MachineConstant::Kind::Int32:
            os << std::format("<int8> {:#06X} ({})", constant.get_int(), constant.get_int());
            break;
        case MachineConstant::Kind::Int64:
            os << std::format("<int8> {:#08X} ({})", constant.get_int(), constant.get_int());
            break;
        case MachineConstant::Kind::Float32:
            os << std::format("<float32> {:#04X} ({})", constant.get_int(), constant.get_fp());
            break;
        case MachineConstant::Kind::Float64:
            os << std::format("<float64> {:#08X} ({})", constant.get_int(), constant.get_fp());
            break;
    }
}

void Printer::print_data(std::ostream &os, const MachineData &data) {
    os << std::format("{}:\n", data.get_name());

    for (const MachineConstant &constant : data.get_data()) {
        os << '\t';
        print_constant(os, constant);
        os << '\n';
    }
}
