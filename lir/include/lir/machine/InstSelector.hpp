//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_INST_SELECTOR_H_
#define LOVELACE_IR_INST_SELECTOR_H_

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Instruction.hpp"
#include "lir/machine/MachFunction.hpp"
#include "lir/machine/MachInst.hpp"

namespace lir {

class InstSelector final {
    using LocalTable = std::unordered_map<const Local*, uint32_t>;
    using RegisterTable = std::unordered_map<uint32_t, Register>;
    
    const Machine& m_mach;
    MachFunction& m_func;

    MachLabel* m_insert = nullptr;

    /// A table for mappings between locals -> stack frame indices.
    LocalTable m_locals = {};

    /// A table for mappings between instruction defs -> vreg numbers.
    RegisterTable m_regs = {};

    /// Returns the ncessary instruction width for the given |type|.
    X64_Size as_size(Type* type) const;

    /// Returns the subregister byte necessary to access all bytes used by the
    /// given |type|. This function always returns 1, 2, 4, or 8.
    uint16_t get_subregister(Type* type) const;

    /// Returns the given |inst| as a new or existing virtual machine register.
    ///
    /// Fails if |inst| is not a defining instruction, i.e. does not produce a
    /// new value.
    Register as_register(const Instruction* inst);

    /// Get a machine register for temporary use with the given register |cls|.
    Register get_temporary(RegisterClass cls);

    /// Converts the given |value| into an X64 instruction operand.
    MachOperand as_operand(const Value* value);

    /// Converts the given |value| into an argument for an X64 call per the
    /// ABI in use by the machine. 
    MachOperand as_argument(const Value* value, uint32_t index) const;

    /// Returns a stringifed assembly comment taken from a print of the given
    /// |inst|.
    std::string get_asm_comment(const Instruction* inst) const;

    /// Create and emit a new instruction with the given |op|, instruction 
    /// |size|, and optional |ops|. The |before_term| flag determines if extra
    /// care should be given to place the new instruction before any existing 
    /// terminators under the current label.
    MachInst& emit(X64_Mnemonic op, X64_Size size = X64_Size::None, 
                   const MachInst::Operands& ops = {}, bool before_terms = false);

    void select(const Instruction* inst);
    void select_abort(const Instruction* inst);
    void select_unreachable(const Instruction* inst);
    void select_load_store(const Instruction* inst);
    void select_pwalk(const Instruction* inst);
    void select_string(const Instruction* inst);
    void select_comparison(const Instruction* inst);
    void select_conditional_jump(const Instruction* inst);
    void select_jump(const Instruction* inst);
    void select_return(const Instruction* inst);
    void select_call(const Instruction* inst);
    void select_iadd(const Instruction* inst);
    void select_fadd(const Instruction* inst);
    void select_isub(const Instruction* inst);
    void select_fsub(const Instruction* inst);
    void select_imul(const Instruction* inst);
    void select_division(const Instruction* inst);
    void select_float_mul_div(const Instruction* inst);
    void select_logic(const Instruction* inst);
    void select_shift(const Instruction* inst);
    void select_not(const Instruction* inst);
    void select_negate(const Instruction* inst);
    void select_extension(const Instruction* inst);
    void select_truncation(const Instruction* inst);
    void select_cast_i2f(const Instruction* inst);
    void select_cast_f2i(const Instruction* inst);
    void select_cast_p2i(const Instruction* inst);
    void select_cast_i2p(const Instruction* inst);
    void select_cast_reinterpret(const Instruction* inst);

public:
    InstSelector(MachFunction& func);

    void run();
};

} // namespace lir

#endif // LOVELACE_IR_INST_SELECTOR_H_
