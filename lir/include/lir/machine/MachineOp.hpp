//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_MACHINE_OP_H_
#define LIR_MACHINE_OP_H_

#include "lir/machine/MachineOperand.hpp"
#include "lir/machine/MachineRegister.hpp"

#include <cstdint>
#include <vector>

namespace lir {

class MachineLabel;

static constexpr uint32_t INTRINSIC_BARRIER = 1u << 31;

/// Reserved machine-independent ops to provide special behavior. 
enum class Intrinsic : uint32_t {
    Husk = INTRINSIC_BARRIER,
    Param,
    Stack_Setup,
    Stack_Reserve,
    Stack_Restore,
    Callsite_Set,
    Callsite_End,
};

class MachineOp final {
public:
    using Operands = std::vector<MachineOperand>;

private:
    uint32_t m_op;
    uint32_t m_pos;
    Operands m_operands;
    std::string m_comment = "";
    MachineLabel *m_parent;
    MachineOp *m_prev = nullptr;
    MachineOp *m_next = nullptr;

public:
    MachineOp(uint32_t op, const Operands &operands,
              MachineLabel *parent = nullptr);

    uint32_t op() const { return m_op; }

    void set_pos(uint32_t pos) { m_pos = pos; }
    uint32_t get_pos() const { return m_pos; }

    /// Test if this is an intrinsic op.
    bool is_intrinsic() const { return m_op >= INTRINSIC_BARRIER; }

    void set_parent(MachineLabel *label) { m_parent = label; }
    const MachineLabel *get_parent() const { return m_parent; }
    MachineLabel *get_parent() { return m_parent; }

    /// Insert this immediately before the given |op| ints parent label.
    /// Fails if |op| does not belong to a label, or if this op has a parent already.
    void insertBefore(MachineOp* op);

    /// Insert this immediately after the given |op| in its parent label.
    /// Fails if |op| does not belong to a label, or if this op has a parent already.
    void insertAfter(MachineOp* op);

    /// Test if this op belongs to a label.
    bool has_parent() const { return m_parent != nullptr; }

    /// Detach this op from its parent label, if it belongs to one.
    void detach();

    /// Returns the function this op ultimately belongs to.
    const MachineFunction *get_function() const;
    MachineFunction *get_function() {
        return const_cast<MachineFunction*>(
            static_cast<const MachineOp*>(this)->get_function());
    }

    void set_prev(MachineOp *op) { m_prev = op; }
    const MachineOp *get_prev() const { return m_prev; }
    MachineOp *get_prev() { return m_prev; }
    
    /// Test if there is an op before this one in the parent label.
    bool has_prev() const { return m_prev != nullptr; }

    void set_next(MachineOp *op) { m_next = op; }
    const MachineOp *get_next() const { return m_next; }
    MachineOp *get_next() { return m_next; }

    /// Test if there is an op after this one in the parent label.
    bool has_next() const { return m_next != nullptr; }

    const Operands &operands() const { return m_operands; }
    Operands &operands() { return m_operands; }

    /// Test if this op has any operands.
    bool has_operands() const { return !m_operands.empty(); }

    /// Returns the number of operands to this op.
    uint32_t num_operands() const { return m_operands.size(); }

    /// Returns the |i|-th operand of this op.
    const MachineOperand &get_operand(uint32_t i) const {
        assert(i < num_operands() && "index out of bounds!");
        return m_operands[i];
    }

    MachineOperand &get_operand(uint32_t i) {
        assert(i < num_operands() && "index out of bounds!");
        return m_operands[i];
    }
    
    /// Append the given |operand| to this op.
    MachineOp &add_operand(const MachineOperand &operand) {
        m_operands.push_back(operand);
        return *this;
    }

    /// Replace the |i|-th operand to this op with |operand|.
    void set_operand(const MachineOperand &operand, uint32_t i) {
        assert(i < num_operands() && "index out of bounds!");
        m_operands[i] = operand;
    }

    /// Test if there are any implicit register operands to this op.
    bool has_implicit_operands() const {
        for (const MachineOperand &op : m_operands) {
            if (op.is_reg() && op.reg().isImplicit())
                return true;
        }

        return false;
    }

    /// Returns the number of implicit register operands to this op.
    uint32_t num_implicit_operands() const {
        uint32_t res = 0;
        for (const MachineOperand &op : m_operands) {
            if (op.is_reg() && op.reg().isImplicit())
                res++;
        }

        return res;
    }

    uint32_t num_explicit_operands() const { return num_operands() - num_implicit_operands(); }

    void set_comment(const std::string &comment) { m_comment = comment; }
    const std::string &get_comment() const { return m_comment; }

    /// Test if this op has a comment.
    bool has_comment() const { return !m_comment.empty(); }

    MachineOp &add_reg(const MachineRegister &reg) {
        add_operand(MachineOperand(reg));
        return *this;
    }

    MachineOp &add_mem(const MachineRegister &base, int32_t offset) {
        add_operand(MachineOperand(Memory { base, offset }));
        return *this;
    }

    MachineOp &add_imm(int64_t imm) {
        add_operand(MachineOperand(imm));
        return *this;
    }

    MachineOp &add_zero() {
        add_operand(MachineOperand(static_cast<int32_t>(0)));
        return *this;
    }

    MachineOp &add_data(MachineData *data) {
        add_operand(MachineOperand(data));
        return *this;
    }

    MachineOp &add_local(MachineLocal* local) {
        add_operand(MachineOperand(local));
        return *this;
    }

    MachineOp &add_function(MachineFunction *func) {
        add_operand(MachineOperand(func));
        return *this;
    }

    MachineOp &add_label(MachineLabel *label) {
        add_operand(MachineOperand(label));
        return *this;
    }

    MachineOp &add_comment(const std::string &comment) {
        m_comment = comment;
        return *this;
    }
};

} // namespace lir

#endif // LIR_MACHINE_OP_H_
