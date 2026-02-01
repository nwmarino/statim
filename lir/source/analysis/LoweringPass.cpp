//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/analysis/LoweringPass.hpp"
#include "lir/machine/MachineFunction.hpp"
#include "lir/machine/MachineOp.hpp"

using namespace lir;

static void lower_constant(const Machine &mach, const Constant *constant, 
                           MachineData::Data &data) {
    if (auto integer = dynamic_cast<const Integer*>(constant)) {
        const uint32_t bits = mach.get_type_size(constant->get_type());
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
    } else if (auto fp = dynamic_cast<const Float*>(constant)) {
        const uint32_t bits = mach.get_type_size(constant->get_type());
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
    } else if (auto null = dynamic_cast<const Null*>(constant)) {
        data.push_back(MachineConstant(MachineConstant::Kind::Int64, 0L));
    } else if (auto string = dynamic_cast<const String*>(constant)) {
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
    } else if (auto aggregate = dynamic_cast<const Aggregate*>(constant)) {
        for (uint32_t i = 0, e = aggregate->num_operands(); i < e; ++i)
            lower_constant(mach, aggregate->get_value(i), data);
    }
}

void LoweringPass::run() {
    for (const Global *global : m_cfg.get_globals()) {
        MachineData::Data bytes = {};
        
        if (global->has_initializer()) {
            lower_constant(m_obj.get_machine(), global->get_initializer(), bytes);
        } else {
            const auto ptr = dynamic_cast<PointerType*>(global->get_type());
            assert(ptr);

            const uint32_t bits = 
                m_obj.get_machine().get_type_size(ptr->get_pointee());
            bytes.push_back(MachineConstant(bits / 8));
        }

        MachineData *MD = new MachineData(
            global->get_name(), 
            bytes,
            global->has_linkage(Global::LinkageType::Public), 
            !global->is_mutable());
        assert(MD);

        m_obj.get_globals().emplace(global->get_name(), MD);
    }

    for (const Function *func : m_cfg.get_functions()) {
        // Empty functions should not be lowered, they should either be
        // resolved at link time or with some library.
        if (func->empty())
            continue;

        MachineFunction *MF = new MachineFunction(&m_obj, func->get_name());
        assert(MF);

        const BasicBlock *curr = func->get_head();
        while (curr) {
            MachineLabel *ML = new MachineLabel(MF);
            assert(ML);

            curr = curr->get_next();
        }

        MF->update_positions();
    }
}
