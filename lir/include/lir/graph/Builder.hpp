//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_BUILDER_H_
#define LOVELACE_IR_BUILDER_H_

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Instruction.hpp"
#include "lir/graph/Type.hpp"

#include <cassert>

namespace lir {

/// An interface for creating instructions in the IR.
class Builder final {
public:
    /// The different insertion modes for new instructions.
    enum class InsertMode : uint8_t {
        Prepend, Append,
    };

private:
    CFG &m_cfg;
    BasicBlock *m_insert = nullptr;
    InsertMode m_mode = InsertMode::Append;

public:
    Builder(CFG &cfg) : m_cfg(cfg) {}

    void set_insert(BasicBlock *block) { m_insert = block; }
    void clear_insert() { m_insert = nullptr; }
    const BasicBlock *get_insert() const { return m_insert; }
    BasicBlock *get_insert() { return m_insert; }

    void set_mode(InsertMode mode) { m_mode = mode; }
    InsertMode get_mode() const { return m_mode; }

    /// Insert the given |inst| at the current insertion point, if it is set.
    void insert(Instruction *inst);

    /// Create a new const instruction defining |value| as an SSA value.
    Const *build_const(Constant *value);

    /// Create a new string const instruction defining |string| as a value.
    Const *build_string(String *string);

    /// Create a new aggregate const instruction defining |agg| as a value.
    Const *build_agg(Aggregate *agg);

    /// Create a new load that reads a value of the given |type| from |ptr|.
    Load *build_load(Type *type, Value *ptr);

    /// Create a new store that writes |value| to |ptr|.
    Store *build_store(Value *value, Value *ptr);

    /// Create a new structure access on the given |ptr| at the given |index|.
    Access *build_access(Type *type, Value *ptr, Value *index);

    /// Create a new pointer offset on the given |ptr| at the given |index|.
    Offptr *build_offptr(Type *type, Value *ptr, Value *index);

    /// Create a new call instruction to the given |callee| with an optional 
    /// list of call |args|.
    Call *build_call(Function *callee, const std::vector<Value*> &args = {});

    /// Create a new ret instruction that optionally returns a |value|.
    Ret *build_ret(Value *value = nullptr);

    /// Build a new jump instruction that jumps to the given |dest|.
    Jump *build_jump(BasicBlock *dest);

    /// Build a new conditional branch instruction, that if |cond| is true,
    /// then branches to |tdest|, otherwise it branches to |fdest|.
    Brif *build_brif(Value *cond, BasicBlock *tdest, BasicBlock *fdest);

    /// Build a phi node for the given |type|.
    Phi *build_phi(Type *type);

    Unop *build_not(Value *value);
    Unop *build_ineg(Value *value);
    Unop *build_fneg(Value *value);

    Binop *build_iadd(Value *lhs, Value *rhs);
    Binop *build_isub(Value *lhs, Value *rhs);
    Binop *build_imul(Value *lhs, Value *rhs);
    Binop *build_sdiv(Value *lhs, Value *rhs);
    Binop *build_udiv(Value *lhs, Value *rhs);
    Binop *build_smod(Value *lhs, Value *rhs);
    Binop *build_umod(Value *lhs, Value *rhs);

    Binop *build_fadd(Value *lhs, Value *rhs);
    Binop *build_fsub(Value *lhs, Value *rhs);
    Binop *build_fmul(Value *lhs, Value *rhs);
    Binop *build_fdiv(Value *lhs, Value *rhs);

    Binop *build_and(Value *lhs, Value *rhs);
    Binop *build_or(Value *lhs, Value *rhs);
    Binop *build_xor(Value *lhs, Value *rhs);

    Binop *build_shl(Value *lhs, Value *rhs);
    Binop *build_shr(Value *lhs, Value *rhs);
    Binop *build_sar(Value *lhs, Value *rhs);

    Cast *build_sext(Type *type, Value *value);
    Cast *build_zext(Type *type, Value *value);
    Cast *build_fext(Type *type, Value *value);

    Cast *build_itrunc(Type *type, Value *value);
    Cast *build_ftrunc(Type *type, Value *value);

    Cast *build_s2f(Type *type, Value *value);
    Cast *build_u2f(Type *type, Value *value);
    Cast *build_f2s(Type *type, Value *value);
    Cast *build_f2u(Type *type, Value *value);

    Cast *build_p2i(Type *type, Value *value);
    Cast *build_i2p(Type *type, Value *value);
    Cast *build_reint(Type *type, Value *value);

    Cmp *build_cmp_ieq(Value *lhs, Value *rhs);
    Cmp *build_cmp_ine(Value *lhs, Value *rhs);

    Cmp *build_cmp_slt(Value *lhs, Value *rhs);
    Cmp *build_cmp_sle(Value *lhs, Value *rhs);
    Cmp *build_cmp_sgt(Value *lhs, Value *rhs);
    Cmp *build_cmp_sge(Value *lhs, Value *rhs);

    Cmp *build_cmp_ult(Value *lhs, Value *rhs);
    Cmp *build_cmp_ule(Value *lhs, Value *rhs);
    Cmp *build_cmp_ugt(Value *lhs, Value *rhs);
    Cmp *build_cmp_uge(Value *lhs, Value *rhs);

    Cmp *build_cmp_feq(Value *lhs, Value *rhs);
    Cmp *build_cmp_fne(Value *lhs, Value *rhs);
    Cmp *build_cmp_flt(Value *lhs, Value *rhs);
    Cmp *build_cmp_fle(Value *lhs, Value *rhs);
    Cmp *build_cmp_fgt(Value *lhs, Value *rhs);
    Cmp *build_cmp_fge(Value *lhs, Value *rhs);
};

} // namespace lir

#endif // LOVELACE_IR_BUILDER_H_
