#ifndef STATIM_SIIR_INSTRUCTION_HPP_
#define STATIM_SIIR_INSTRUCTION_HPP_

#include "siir/user.hpp"

namespace stm {

class BasicBlock;
class Function;

class Instruction : public User {
protected:
    BasicBlock* m_parent;
    Instruction* m_prev = nullptr;
    Instruction* m_next = nullptr;

    Instruction(BasicBlock* parent);

public:
    virtual ~Instruction() = default;

    /// \returns The stringified version of this instructions opcode.
    virtual std::string opcode_to_string() const;

    /// \returns `true` if this is a call instruction.
    virtual bool is_call() const { return false; }

    /// \returns `true` if this is a return instruction.
    virtual bool is_return() const { return false; }

    /// \returns `true` if this is a terminating instruction.
    virtual bool is_terminator() const { return false; }

    /// \returns `true` if this is a unary operation.
    virtual bool is_unop() const { return false; }

    /// \returns `true` if this is a binary operation.
    virtual bool is_binop() const { return false; }

    /// \returns `true` if this is a casting instruction.
    virtual bool is_cast() const { return false; }

    const BasicBlock* get_parent() const { return m_parent; }
    BasicBlock* get_parent() { return m_parent; }

    /// Clear the link to the parent block of this instruction.
    void clear_parent() { m_parent = nullptr; }

    const Function* get_function() const;
    Function* get_function() {
        return const_cast<Function*>(
            static_cast<const Instruction*>(this)->get_function());
    }

    /// Append this instruction to a new parent block \p parent. Assumes that
    /// this instruction is unlinked and free-floating.
    void append_to(BasicBlock* parent);

    /// Insert this instruction into the position before \p inst.
    void insert_before(Instruction* inst);

    /// Insert this instruction into the position after \p inst.
    void insert_after(Instruction* inst);

    /// Detach this instruction from its parent block. Does not destroy the
    /// instruction.
    void detach();

    const Instruction* prev() const { return m_prev; }
    Instruction* prev() { return m_prev; }

    const Instruction* next() const { return m_next; }
    Instruction* next() { return m_next; }

    void set_prev(Instruction* inst) { m_prev = inst; }
    void set_next(Instruction* inst) { m_next = inst; }
};

class StoreInst final : public Instruction {
    Value* m_src;
    Value* m_dst;
    u32 m_align;
};

class LoadInst final : public Instruction {
    Value* m_src;
    u32 m_align;
};

class BrifInst final : public Instruction {
    Value* m_cond;
    BasicBlock* m_tdest;
    BasicBlock* m_fdest;
};

class JmpInst final : public Instruction {
    BasicBlock* m_dest;
};

class RetInst final : public Instruction {
    Value* m_value;
};

class CallInst final : public Instruction {
    Function* m_callee;
    std::vector<Value*> m_args;
};

class CmpInst final : public Instruction {
public:
    enum Predicate : u8 {
        CMP_Eq, CMP_Ne,
        CMP_Oeq, CMP_One,
        CMP_Uneq, CMP_Unne,
        CMP_Slt, CMP_Sle, CMP_Sgt, CMP_Sge,
        CMP_Ult, CMP_Ule, CMP_Ugt, CMP_Uge,
        CMP_Olt, CMP_Ole, CMP_Ogt, CMP_Oge,
        CMP_Unlt, CMP_Unle, CMP_Ungt, CMP_Unge,
    };

private:
    Predicate m_pred;
    Value* m_left;
    Value* m_right;
};

class BinopInst final : public Instruction {
public:
    enum Operator : u8 {
        BINOP_Add, BINOP_FAdd,
        BINOP_Sub, BINOP_FSub,
        BINOP_Mul, BINOP_FMul,
        BINOP_SDiv, BINOP_UDiv, BINOP_FDiv,
        BINOP_SRem, BINOP_URem, BINOP_FRem,
        BINOP_And, BINOP_Or, BINOP_Xor,
        BINOP_Shl, BINOP_Shr, BINOP_AShr,
    };

private:
    Operator m_op;
    Value* m_left;
    Value* m_right;
};

class UnopInst final : public Instruction {
public:
    enum Operator : u8 {
        UNOP_Not,
        UNOP_Neg, UNOP_FNeg,
        UNOP_SExt, UNOP_ZExt, UNOP_FExt,
        UNOP_Trunc, UNOP_FTrunc,
        UNOP_SI2FP, UNOP_UI2FP,
        UNOP_FP2SI, UNOP_FP2UI,
        UNOP_P2I, UNOP_I2P,
        UNOP_Reint,
    };

private:
    Operator m_op;
    Value* m_value;
};

} // namespace stm

#endif // STATIM_SIIR_INSTRUCTION_HPP_
