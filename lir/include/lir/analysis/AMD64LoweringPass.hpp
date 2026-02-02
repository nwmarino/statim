//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_AMD64_LOWERING_PASS_H_
#define LIR_AMD64_LOWERING_PASS_H_

#include "lir/analysis/LoweringPass.hpp"
#include "lir/machine/AMD64.hpp"
#include "lir/machine/MachineOp.hpp"

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
