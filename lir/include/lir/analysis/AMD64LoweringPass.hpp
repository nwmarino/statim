//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_AMD64_LOWERING_PASS_H_
#define LIR_AMD64_LOWERING_PASS_H_

#include "lir/analysis/LoweringPass.hpp"
#include "lir/machine/AMD64.hpp"
#include "lir/machine/MachineOp.hpp"

#include <array>
#include <unordered_map>

namespace lir {

class AMD64LoweringPass final : public LoweringPass {
    using LocalTable = std::unordered_map<const Local*, MachineLocal*>;
    using DefTable = std::unordered_map<uint32_t, Register>;

    MachineFunction *m_func = nullptr;
    MachineLabel *m_insert = nullptr;

    /// A table from LIR Locals -> MIR stack locals.
    LocalTable m_locals = {};

    /// A table from LIR Instructions Defs -> MIR virtual registers.
    DefTable m_defs = {};

public:
    AMD64LoweringPass(CFG &cfg, MachineObject &obj) : LoweringPass(cfg, obj) {}

    void run() override;

private:
    /// Returns the AMD64 register byte offset for the given scalar |type|.
    uint8_t get_subreg_byte(const Type *type) const;

    /// Returns the sized AMD64 op for the given scalar integer |type| from
    /// the general purpose |gp| ops.
    ///
    /// The ops should be in the order of { 8, 16, 32, 64 } precisions.
    AMD64_Op get_sized_op(const Type *type, const std::array<AMD64_Op, 4> &gp);

    AMD64_Op getSizedOp(const Type* type, const std::array<AMD64_Op, 2>& fp);

    /// Returns the sized AMD64 op for the given scalar |type|, from either
    /// the general purpose |gp| ops, or floating point |fp| ops.
    ///
    /// The general purpose ops should be in the order of { 8, 16, 32, 64 },
    /// precisions and floating points { single, double } precisions.
    AMD64_Op get_sized_op(const Type *type, const std::array<AMD64_Op, 4> &gp, 
                          const std::array<AMD64_Op, 2> &fp);

    /// Returns the AMD64 MOVE op for the given scalar |type|.
    AMD64_Op get_move_op(const Type *type);

    /// Returns the AMD64 CMP op for the given scalar |type|.
    AMD64_Op get_cmp_op(const Type *type);

    AMD64_Op getOpIAdd(const Type* type);
    AMD64_Op getOpISub(const Type* type);
    AMD64_Op getOpIMul(const Type* type);
    AMD64_Op getOpSDiv(const Type* type);
    AMD64_Op getOpUDiv(const Type* type);

    AMD64_Op getOpAnd(const Type* type);
    AMD64_Op getOpOr(const Type* type);
    AMD64_Op getOpXor(const Type* type);

    AMD64_Op getOpShl(const Type* type);
    AMD64_Op getOpShr(const Type* type);
    AMD64_Op getOpSar(const Type* type);

    AMD64_Op getOpFAdd(const Type* type);
    AMD64_Op getOpFSub(const Type* type);
    AMD64_Op getOpFMul(const Type* type);
    AMD64_Op getOpFDiv(const Type* type);

    AMD64_Op getOpNot(const Type* type);
    AMD64_Op getOpINeg(const Type* type);

    AMD64_Op getOpSS2SI(const Type* type);
    AMD64_Op getOpSD2SI(const Type* type);
    AMD64_Op getOpSS2UI(const Type* type);
    AMD64_Op getOpSD2UI(const Type* type);

    /// Create and return a new virtual register of the given |cls|.
    Register create_vreg(RegisterClass cls);

    /// Get the virtual register lowered from the given |def|.
    Register get_vreg_from_def(const Instruction *inst);

    /// Convert the given |value| to a machine operand, where possible.
    MachineOperand to_operand(const Value *value);

    /// Emit a new instruction to the back of the current label with the given 
    /// |op| and |operands|.
    MachineOp &emit(uint32_t op, const MachineOp::Operands &operands = {});

    /// Construct the stack frame for the given |func|.
    void construct_stack_frame(const Function *func);

    /// Lower the given |inst| into the current label.
    void lower_inst(const Instruction *inst);

    void lower_const(const Const *C);
    void lower_load(const Load *L);
    void lower_store(const Store *S);
    void lower_access(const Access *A);
    void lower_extract(const Extract *E);
    void lower_offptr(const Offptr *O);
    void lower_call(const Call *C);
    void lower_ret(const Ret *R);
    void lower_jump(const Jump *J);
    void lower_brif(const Brif *B);
    void lower_phi(const Phi *P);
    void lower_unop(const Unop *U);
    void lower_binop(const Binop *B);
    void lower_cast(const Cast *C);
    void lower_cmp(const Cmp *C);
};

} // namespace lir

#endif // LIR_AMD64_LOWERING_PASS_H_
