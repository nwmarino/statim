//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/analysis/LoweringPass.h"
#include "lir/machine/MachineFunction.h"

using namespace lir;

LoweringPass::LoweringPass(CFG &cfg, MachineObject &obj)
  : Pass(cfg), m_mach(cfg.get_machine()), m_obj(obj) {}

void LoweringPass::lower_constant(const Constant* C, std::vector<MachineConstant>& data) const {
    if (auto integer = dynamic_cast<const Integer*>(C)) {
        const uint32_t bits = m_mach.get_type_size(C->get_type());
        const int64_t value = integer->get_value();
        
        MachineConstant::Kind kind;
        switch (bits) {
            case 8:
                kind = MachineConstant::Kind::Int8;
                break;
            case 16:
                kind = MachineConstant::Kind::Int16;
                break;
            case 32:
                kind = MachineConstant::Kind::Int32;
                break;
            case 64:
                kind = MachineConstant::Kind::Int64;
                break;
            default:
                assert(false && "invalid integer bit width!");
        }

        data.push_back(MachineConstant(kind, value));
    } else if (auto fp = dynamic_cast<const Float*>(C)) {
        const uint32_t bits = m_mach.get_type_size(C->get_type());
        const double value = fp->get_value();

        MachineConstant::Kind kind;
        switch (bits) {
            case 32:
                kind = MachineConstant::Kind::Float32;
                break;
            case 64:
                kind = MachineConstant::Kind::Float64;
                break;
            default:
                assert(false && "invalid floating point bit width!");
        }

        data.push_back(MachineConstant(kind, value));
    } else if (auto null = dynamic_cast<const Null*>(C)) {
        data.push_back(MachineConstant(MachineConstant::Kind::Int64, 0L));
    } else if (auto string = dynamic_cast<const String*>(C)) {
        const std::string &value = string->get_value();
        const MachineConstant::Kind kind = MachineConstant::Kind::Int8;

        for (uint32_t i = 0, e = value.size(); i < e; ++i) {
            const int64_t ch = static_cast<long>(value[i]);
            switch (value[i]) {
                case '\\':
                    data.push_back(MachineConstant(kind, (long) '\\'));
                    data.push_back(MachineConstant(kind, (long) '\\'));
                    break;
                case '\'':
                    data.push_back(MachineConstant(kind, (long) '\\'));
                    data.push_back(MachineConstant(kind, (long) '\''));
                    break;
                case '\"':
                    data.push_back(MachineConstant(kind, (long) '\\'));
                    data.push_back(MachineConstant(kind, (long) '"'));
                    break;
                case '\n':
                    data.push_back(MachineConstant(kind, (long) '\\'));
                    data.push_back(MachineConstant(kind, (long) 'n'));
                    break;
                case '\t':
                    data.push_back(MachineConstant(kind, (long) '\\'));
                    data.push_back(MachineConstant(kind, (long) 't'));
                    break;
                case '\r':
                    data.push_back(MachineConstant(kind, (long) '\\'));
                    data.push_back(MachineConstant(kind, (long) 'r'));
                    break;
                case '\b':
                    data.push_back(MachineConstant(kind, (long) '\\'));
                    data.push_back(MachineConstant(kind, (long) 'b'));
                    break;
                case '\0':
                    data.push_back(MachineConstant(kind, (long) '\\'));
                    data.push_back(MachineConstant(kind, (long) '0'));
                    break;
                default:
                    data.push_back(MachineConstant(kind, ch));
                    break;
            }
        }
    } else if (auto aggregate = dynamic_cast<const Aggregate*>(C)) {
        for (uint32_t i = 0, e = aggregate->num_operands(); i < e; ++i)
            lower_constant(aggregate->get_value(i), data);
    }
}
