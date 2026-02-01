//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_MACHINE_OPERAND_H_
#define LIR_MACHINE_OPERAND_H_

#include "lir/machine/MachineData.hpp"
#include "lir/machine/MachineRegister.hpp"

#include <cstdint>

namespace lir {

class MachineLocal;
class MachineConstant;
class MachineFunction;
class MachineLabel;

/// Represents a memory access at a base + offset.
struct Memory final {
    MachineRegister base;
    int32_t offset;
};

/// An operand to a MachineOp.
class MachineOperand final {
public:
    /// The different kinds of operands.
    enum class Kind : uint32_t {
        Register,
        Memory,
        Immediate,
        Data,
        Local,
        Function,
        Label,
    };

private:
    Kind m_kind;
    union {
        /// Kind::Register operands.
        MachineRegister m_reg;

        /// Kind::Memory operands.
        Memory m_mem;

        /// Kind::Immediate operands.
        int32_t m_imm;

        /// Kind::Data operands.
        MachineData *m_data;

        /// Kind::Local operands.
        MachineLocal *m_local;

        /// Kind::Function operands.
        MachineFunction *m_func;

        /// Kind::Label operands.
        MachineLabel *m_label;
    };

public:
    MachineOperand(const MachineRegister &reg) 
      : m_kind(Kind::Register), m_reg(reg) {}
    
    MachineOperand(const Memory &mem) 
      : m_kind(Kind::Memory), m_mem(mem) {}

    MachineOperand(int32_t imm) 
      : m_kind(Kind::Immediate), m_imm(imm) {}

    MachineOperand(MachineData *data) 
      : m_kind(Kind::Data), m_data(data) {}

    MachineOperand(MachineLocal *local) 
      : m_kind(Kind::Local), m_local(local) {}

    MachineOperand(MachineFunction *func)
      : m_kind(Kind::Function), m_func(func) {}

    MachineOperand(MachineLabel *label)
      : m_kind(Kind::Label), m_label(label) {}

    Kind kind() const { return m_kind; }

    /// Test if this is a register operand.
    bool is_reg() const { return m_kind == Kind::Register; }

    /// Test if this is a memory operand.
    bool is_mem() const { return m_kind == Kind::Memory; }

    /// Test if this is an immediate operand.
    bool is_imm() const { return m_kind == Kind::Immediate; }
    
    /// Test if this is a data operand.
    bool is_data() const { return m_kind == Kind::Data; }

    /// Test if this is a stack local operand.
    bool is_local() const { return m_kind == Kind::Local; }

    /// Test if this is a function symbol operand.
    bool is_function() const { return m_kind == Kind::Function; }

    /// Test if this is a label operand.
    bool is_label() const { return m_kind == Kind::Label; }

    const MachineRegister &reg() const { 
        assert(is_reg() && "this is not a register operand!");
        return m_reg; 
    }

    const Memory &mem() const {
        assert(is_mem() && "this is not a memory operand!");
        return m_mem;
    }

    const int32_t& imm() const {
        assert(is_imm() && "this is not an immediate operand!");
        return m_imm;
    }

    const MachineData *data() const {
        assert(is_data() && "this is not a data operand!");
        return m_data; 
    }

    const MachineLocal *local() const {
        assert(is_local() && "this is not a local operand!");
        return m_local;
    }

    const MachineFunction *function() const {
        assert(is_function() && "this is not a function operand!");
        return m_func;
    }

    const MachineLabel *label() const {
        assert(is_label() && "this is not a label operand!");
        return m_label;
    }
};

} // namespace lir

#endif // LIR_MACHINE_OPERAND_H_
