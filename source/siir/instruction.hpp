#ifndef STATIM_SIIR_INSTRUCTION_HPP_
#define STATIM_SIIR_INSTRUCTION_HPP_

#include "siir/constant.hpp"
#include "siir/user.hpp"
#include <initializer_list>

namespace stm {

namespace siir {

class BasicBlock;
class Function;

class Instruction : public User {
protected:
    BasicBlock* m_parent;
    Instruction* m_prev = nullptr;
    Instruction* m_next = nullptr;

    Instruction(std::initializer_list<Value*> operands, BasicBlock* parent)
        : User(operands), m_parent(parent) {}

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

/// Represents a `const` instruction.
class ConstInst final : public Instruction {
    Constant* m_const;

    ConstInst(Constant* constant, const std::string& name, 
              BasicBlock* append_to);

public:
    ~ConstInst() override;

    /// Create a new constant instruction for value \p constant. 
    static ConstInst* create(Constant* constant, const std::string& name = "",
                             BasicBlock* append_to = nullptr);

    const Constant* get_constant() const { return m_const; }
    Constant* get_constant() { return m_const; }
};

/// Represents a `store` instruction.
class StoreInst final : public Instruction {
    Value* m_value;
    Value* m_dst;
    u32 m_align;

    StoreInst(Value* value, Value* dst, u32 align, BasicBlock* append_to);

public:
    ~StoreInst() override;

    /// Create a new store instruction with value \p value, destination \p dst,
    /// and alignment \p align.
    static StoreInst* create(Value* value, Value* dst, u32 align, 
                             BasicBlock* append_to = nullptr);

    const Value* get_value() const { return m_value; }
    Value* get_value() { return m_value; }

    const Value* get_destination() const { return m_dst; }
    Value* get_destination() { return m_dst; }

    u32 get_alignment() const { return m_align; }

    void set_value(Value* value) { m_value = value; }
    void set_destination(Value* value) { m_dst = value; }
    void set_alignment(u32 align) { m_align = align; }
};

/// Represents a `load` instruction.
class LoadInst final : public Instruction {
    Value* m_src;
    u32 m_align;

    LoadInst(Value* src, u32 align, const Type* type, const std::string& name,
             BasicBlock* append_to);

public:
    ~LoadInst() override;

    /// Create a new load instruction with source \p src, and alignment 
    /// \p align.
    static LoadInst* create(Value* src, u32 align, const Type* type,
                            const std::string& name = "", 
                            BasicBlock* append_to = nullptr);

    const Value* get_source() const { return m_src; }
    Value* get_source() { return m_src; }
    
    u32 get_alignment() const { return m_align; }

    void set_source(Value* value) { m_src = value; }
    void set_alignment(u32 align) { m_align = align; }
};

/// Represents a `sel` instruction.
class SelectInst final : public Instruction {
    Value* m_cond;
    Value* m_tval;
    Value* m_fval;

    SelectInst(Value* cond, Value* tval, Value* fval, const std::string& name, 
               BasicBlock* append_to);

public:
    ~SelectInst() override;

    /// Create a new select instruction with the condition \p cond and
    /// potential values \p tval, \p fval.
    static SelectInst* create(Value* cond, Value* tval, Value* fval, 
                              const std::string& name = "",
                              BasicBlock* append_to = nullptr);

    const Value* get_condition() const { return m_cond; }
    Value* get_condition() { return m_cond; }

    const Value* get_true_value() const { return m_tval; }
    Value* get_true_value() { return m_tval; }

    const Value* get_false_value() const { return m_fval; }
    Value* get_false_value() { return m_fval; }

    void set_condition(Value* value) { m_cond = value; }
    void set_true_value(Value* value) { m_tval = value; }
    void set_false_value(Value* value) { m_fval = value; }

    /// Swap the true and false values.
    void swap_values() {
        Value* tmp = m_tval;
        m_tval = m_fval;
        m_fval = tmp;
    }
};

/// Represents a terminating `brif` instruction.
///
/// * Does not produce a value.
///
class BrifInst final : public Instruction {
    Value* m_cond;
    Value* m_tdst;
    Value* m_fdst;

    BrifInst(Value* cond, Value* tdst, Value* fdst, BasicBlock* append_to);

public:
    ~BrifInst() override;

    /// Create a new branch-if instruction with the condition \p cond and
    /// destination blocks \p tdst, \p fdst.
    static BrifInst* create(Value* cond, Value* tdst, Value* fdst, 
                            BasicBlock* append_to = nullptr);

    bool is_terminator() const override { return true; }

    const Value* get_condition() const { return m_cond; }
    Value* get_condition() { return m_cond; }

    const Value* get_true_dest() const { return m_tdst; }
    Value* get_true_dest() { return m_tdst; }

    const Value* get_false_dest() const { return m_fdst; }
    Value* get_false_dest() { return m_fdst; }

    void set_condition(Value* value) { m_cond = value; }
    void set_true_dest(Value* value) { m_tdst = value; }
    void set_false_dest(Value* value) { m_fdst = value; }

    /// Swap the true and false destinations.
    void swap_destinations() {
        Value* tmp = m_tdst;
        m_tdst = m_fdst;
        m_fdst = tmp;
    }
};

/// Represents a terminating `jmp` instruction.
///
/// * Does not produce a value.
///
class JmpInst final : public Instruction {
    Value* m_dst;

    JmpInst(Value* dst, BasicBlock* append_to);

public:
    ~JmpInst() override;

    /// Create a new jump instruction with the destination block \p dst.
    static JmpInst* create(Value* dst, BasicBlock* append_to = nullptr);

    bool is_terminator() const override { return true; }

    const Value* get_destination() const { return m_dst; }
    Value* get_destination() { return m_dst; }

    void set_destination(Value* value) { m_dst = value; }
};

/// Represents a terminating `ret` instruction.
///
/// * Does not produce a value.
///
class RetInst final : public Instruction {
    Value* m_value;

    RetInst(Value* value, BasicBlock* append_to);

public:
    ~RetInst() override;

    /// Create a new return instruction with the return value \p value.
    static RetInst* create(Value* value, BasicBlock* append_to = nullptr);

    bool is_return() const override { return true; }

    bool is_terminator() const override { return true; }

    const Value* get_return_value() const { return m_value; }
    Value* get_return_value() { return m_value; }
};

/// Represents a terminating `abort` instruction.
///
/// * Does not produce a value.
///
class AbortInst final : public Instruction {
    AbortInst(BasicBlock* append_to);

public:
    /// Create a new abort instruction.
    static AbortInst* create(BasicBlock* append_to = nullptr);

    bool is_terminator() const override { return true; }
};

/// Represents a terminating `unreachable` instruction.
///
/// * Does not produce a value.
///
class UnreachableInst final : public Instruction {
    UnreachableInst(BasicBlock* append_to);

public:
    /// Create a new unreachable instruction.
    static UnreachableInst* create(BasicBlock* append_to = nullptr);

    bool is_terminator() const override { return true; }
};

class CallInst final : public Instruction {
    Value* m_callee;
    std::vector<Value*> m_args;

    CallInst(Value* callee, const std::vector<Value*>& args, 
             const Type* type, const std::string& name, BasicBlock* append_to);

public:
    ~CallInst() override;

    /// Create a new call instruction to Value \p callee with arguments \p args.
    static CallInst* create(Value* callee, const std::vector<Value*>& args,
                            const Type* type, const std::string& name = "", 
                            BasicBlock* append_to = nullptr);

    bool is_call() const override { return true; }

    const Value* get_callee() const { return m_callee; }
    Value* get_callee() { return m_callee; }
    void set_callee(Value* value) { m_callee = value; }

    const std::vector<Value*>& args() const { return m_args; }

    const Value* get_arg(u32 i) const {
        assert(i <= num_args());
        return m_args[i];
    }

    Value* get_arg(u32 i) {
        assert(i <= num_args());
        return m_args[i];
    }

    u32 num_args() const { return m_args.size(); }

    bool has_args() const { return !m_args.empty(); }
};

/// Represents a comparison instruction.
class CmpInst final : public Instruction {
public:
    /// Possible comparison predicates.
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

    CmpInst(Predicate pred, Value* left, Value* right, const Type* type, 
            const std::string& name, BasicBlock* append_to);

public:
    ~CmpInst() override;

    /// Create a new comparison instruction with predicate \p pred and operands 
    /// \p left, \p right.
    static CmpInst* create(Predicate pred, Value* left, Value* right,
                           const Type* type, const std::string& name = "", 
                           BasicBlock* append_to = nullptr);

    Predicate get_predicate() const { return m_pred; }

    const Value* get_lhs_value() const { return m_left; }
    Value* get_lhs_value() { return m_left; }

    const Value* get_rhs_value() const { return m_right; }
    Value* get_rhs_value() { return m_right; }

    void set_lhs_value(Value* value) { m_left = value; }
    void set_rhs_value(Value* value) { m_right = value; }
};

/// Represents a binary operation instruction.
class BinopInst final : public Instruction {
public:
    enum Ops : u8 {
        BINOP_Add, BINOP_FAdd,
        BINOP_Sub, BINOP_FSub,
        BINOP_Mul, BINOP_FMul,
        BINOP_SDiv, BINOP_UDiv, BINOP_FDiv,
        BINOP_SRem, BINOP_URem, BINOP_FRem,
        BINOP_And, BINOP_Or, BINOP_Xor,
        BINOP_Shl, BINOP_Shr, BINOP_AShr,
    };

private:
    Ops m_op;
    Value* m_left;
    Value* m_right;

    BinopInst(Ops op, Value* left, Value* right, const Type* type, 
              const std::string& name, BasicBlock* append_to);

public:
    ~BinopInst() override;

    /// Create a new binary operation with operator \p op and operands \p left,
    /// \p right.
    static BinopInst* create(Ops op, Value* left, Value* right,
                             const Type* type, const std::string& name = "", 
                             BasicBlock* append_to = nullptr);

    Ops get_operator() const { return m_op; }

    const Value* get_lhs_value() const { return m_left; }
    Value* get_lhs_value() { return m_left; }

    const Value* get_rhs_value() const { return m_right; }
    Value* get_rhs_value() { return m_right; }

    void set_lhs_value(Value* value) { m_left = value; }
    void set_rhs_value(Value* value) { m_right = value; }
};

/// Represents a unary operation instruction.
class UnopInst final : public Instruction {
public:
    enum Ops : u8 {
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
    Ops m_op;
    Value* m_value;

    UnopInst(Ops op, Value* value, const Type* type, const std::string& name, 
             BasicBlock* append_to);

public:
    ~UnopInst() override;

    /// Create a new unary operation with operator \p op on value \p value.
    static UnopInst* create(Ops op, Value* value, const Type* type,
                            const std::string& name = "", 
                            BasicBlock* append_to = nullptr);

    bool is_cast() const override { return m_op >= UNOP_SExt; }

    Ops get_operator() const { return m_op; }

    const Value* get_value() const { return m_value; }
    Value* get_value() { return m_value; }

    void set_value(Value* value) { m_value = value; }
};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_INSTRUCTION_HPP_
