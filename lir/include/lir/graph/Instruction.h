//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_INSTRUCTION_H_
#define LIR_INSTRUCTION_H_

#include "lir/graph/Constant.h"
#include "lir/graph/Debug.h"
#include "lir/graph/User.h"
#include "lir/graph/Value.h"

#include <cstdint>

namespace lir {

class BasicBlock;

/// Base class for all instructions in the IR.
class Instruction : public User {
public:
    using Operands = std::vector<Value*>;

protected:
    BasicBlock *m_parent;
    Instruction *m_prev = nullptr;
    Instruction *m_next = nullptr;
    DebugLoc* m_loc = nullptr;
    uint32_t m_def;

    Instruction(Type *type, BasicBlock *parent, uint32_t def = 0, 
                const Operands &ops = {}, DebugLoc* loc = nullptr);

public:
    virtual ~Instruction() = default;

    Instruction(const Instruction&) = delete;
    void operator=(const Instruction&) = delete;

    Instruction(Instruction&&) noexcept = delete;
    void operator=(Instruction&&) noexcept = delete;

    /// Test if this instruction is a definition, i.e. produces an SSA value.
    bool is_def() const { return m_def != 0; }

    /// Returns the SSA value that this instruction defines, if it is a def.
    uint32_t def() const {
        assert(is_def() && "instruction is not a def!");
        return m_def;
    }

    /// Returns the |i|-th operand of this instruction.
    const Value *get_operand(uint32_t i) const {
        assert(i < num_operands() && "index out of bounds!");
        return m_operands[i]->get_value();
    }

    Value *get_operand(uint32_t i) {
        assert(i < num_operands() && "index out of bounds!");
        return m_operands.at(i)->get_value();
    }

    /// Set the debug location of this instruction to |loc|.
    void set_location(DebugLoc* loc) { m_loc = loc; }

    /// Clear the debug location of this instruction, if it has one.
    void clear_location() { m_loc = nullptr; }

    /// Returns the debug location of this instruction if it has one, and null
    /// otherwise.
    const DebugLoc* get_location() const { return m_loc; }
    DebugLoc* get_location() { return m_loc; }

    /// Test if this instruction has a debug location.
    bool has_location() const { return m_loc != nullptr; }

    void set_parent(BasicBlock *block) { m_parent = block; }
    const BasicBlock *get_parent() const { return m_parent; }
    BasicBlock *get_parent() { return m_parent; }

    /// Test if this instruction has a parent block.
    bool has_parent() const { return m_parent != nullptr; }

    /// Detach this instruction from its parent basic block, removing it from
    /// the list of instructions.
    /// This does not free any memory allocated for this instruction.
    void detach();

    /// Prepend this instruction to the front of the given |block|.
    void prepend_to(BasicBlock *block);

    /// Append this instruction to the back of the given |block|.
    void append_to(BasicBlock *block);

    /// Insert this instruction before the given |inst|. 
    /// Fails if this instruction already belongs to a basic block.
    void insert_before(Instruction *inst);

    /// Insert this instruction after the given |inst|. 
    /// Fails if this instruction already belongs to a basic block.
    void insert_after(Instruction *inst);

    void set_prev(Instruction *inst) { m_prev = inst; }

    /// Returns the instruction that appears before this one in the parent block.
    const Instruction *get_prev() const { return m_prev; }
    Instruction *get_prev() { return m_prev; }

    void set_next(Instruction *inst) { m_next = inst; }

    /// Returns the instruction that appears after this one in the parent block.
    const Instruction *get_next() const { return m_next; }
    Instruction *get_next() { return m_next; }

    /// Test if this instruction is trivially dead.
    ///
    /// An instruction is considered trivially dead if it produces a value
    /// which is unused and has no otherwise side effects.
    ///
    /// The "trivial" part simply means that the instruction can be considered
    /// dead based purely on local context, and without any kind of propogation
    /// or peephole optimizations.
    bool is_trivially_dead() const;
    
    /// Test if this instruction is terminator, i.e. terminates control flow
    /// from a basic block.
    virtual bool is_terminator() const { return false; }
};

/// A const instruction yields a constant value as an SSA value.
class Const final : public Instruction {
    friend class Builder;

private:
    Const(Type *type, BasicBlock *parent, uint32_t def, Constant *value)
      : Instruction(type, parent, def, { value }) {}

public:
    /// Returns the constant that this instruction uses.
    const Constant *get_value() const { 
        return static_cast<const Constant*>(get_operand(0));
    }
    
    Constant *get_value() { 
        return static_cast<Constant*>(get_operand(0)); 
    }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A load instruction produces a value by reading from a given address.
class Load final : public Instruction {
    friend class Builder;

    uint32_t m_align;

    Load(Type *type, BasicBlock *parent, uint32_t def, Value *ptr, 
         uint32_t align)
      : Instruction(type, parent, def, { ptr }), m_align(align) {}

public:
    /// Returns the address that this load uses.
    const Value *get_addr() const { return get_operand(0); }
    Value *get_addr() { return get_operand(0); }

    /// Returns the alignment of this load.
    uint32_t get_align() const { return m_align; }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A store instruction writes a value to a given address.
class Store final : public Instruction {
    friend class Builder;

    uint32_t m_align;

    Store(BasicBlock *parent, Value *value, Value *ptr, uint32_t align)
      : Instruction(nullptr, parent, 0, { value, ptr }), m_align(align) {}

public:
    /// Returns the value that this store uses.
    const Value *get_value() const { return get_operand(0); }
    Value *get_value() { return get_operand(0); }

    /// Returns the address that this store uses.
    const Value *get_addr() const { return get_operand(1); }
    Value *get_addr() { return get_operand(1); }

    /// Returns the alignment of this store.
    uint32_t get_align() const { return m_align; }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// An access instruction yields the pointer to a field of an addressed struct
/// by a lone, numeric index.
class Access final : public Instruction {
    friend class Builder;

    Access(Type *type, BasicBlock *parent, uint32_t def, Value *ptr, 
           Value *index)
      : Instruction(type, parent, def, { ptr, index }) {}

public:
    /// Returns the base structure value of this access.
    const Value *get_base() const { return get_operand(0); }
    Value *get_base() { return get_operand(0); }

    /// Returns the index of this access.
    const Value *get_index() const { return get_operand(1); }
    Value *get_index() { return get_operand(1); }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// An offptr instruction performs pointer arithmetic by adding an offset 
/// to a base pointer based on a numeric index.
class Offptr final : public Instruction {
    friend class Builder;

    Offptr(Type *type, BasicBlock *parent, uint32_t def, Value *ptr, 
          Value *index)
      : Instruction(type, parent, def, { ptr, index }) {}

public:
    /// Returns the base structure value of this instruction.
    const Value *get_base() const { return get_operand(0); }
    Value *get_base() { return get_operand(0); }

    /// Returns the index of this instruction.
    const Value *get_index() const { return get_operand(1); }
    Value *get_index() { return get_operand(1); }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A call instruction performs a function call with a list of arguments.
class Call final : public Instruction {
    friend class Builder;

    Call(Type *type, BasicBlock *parent, uint32_t def, const Operands &args)
      : Instruction(type, parent, def, args) {}

public:
    /// Returns the callee value of this function call.
    const Value *get_callee() const { return get_operand(0); }
    Value *get_callee() { return get_operand(0); }

    /// Returns the number of arguments in this function call.
    uint32_t num_args() const { return num_operands() - 1; }

    /// Test if there are any arguments in this function call.
    bool has_args() const { return num_args() != 0; }

    /// Returns the |i|-th argument of this function call.
    const Value *get_arg(uint32_t i) const {
        assert(i < num_args() && "index out of bounds!");
        return get_operand(i + 1);
    }

    Value *get_arg(uint32_t i) {
        assert(i < num_args() && "index out of bounds!");
        return get_operand(i + 1);
    }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A ret instruction returns control flow from a function, potentially
/// containing a number of values as results.
class Ret final : public Instruction {
    friend class Builder;

    Ret(BasicBlock *parent, Value *value)
      : Instruction(nullptr, parent, 0, { value }) {}

public:
    /// Returns the value that this instruction returns, if there is one.
    const Value *get_value() const { return get_operand(0); }
    Value *get_value() { return get_operand(0); }

    /// Test if this ret contains a value.
    bool has_value() const { return num_operands() != 0; }

    bool is_terminator() const override { return true; }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A jump instruction is a terminator that sends control flow to the sole
/// successor of its parent block.
class Jump final : public Instruction {
    friend class Builder;

    Jump(BasicBlock *parent) : Instruction(nullptr, parent) {}

public:
    /// Returns the block this instruction will jump to.
    /// If this instruction does not belong to a block, then null is returned.
    const BasicBlock *get_dest() const;
    BasicBlock *get_dest() {
        return const_cast<BasicBlock*>(
            static_cast<const Jump*>(this)->get_dest());
    }

    bool is_terminator() const override { return true; }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A branch-if instruction is a terminator that sends control flow to one of 
/// two successors of its parent block based on a condition.
class Brif final : public Instruction {
    friend class Builder;

    Brif(BasicBlock *parent, Value *cond)
      : Instruction(nullptr, parent, 0, { cond }) {}

public:
    /// Returns the conditional value of this instruction.
    const Value *get_cond() const { return get_operand(0); }
    Value *get_cond() { return get_operand(0); }

    /// Returns the block this instruction branches to if the condition is true.
    /// If this instruction does not belong to a block, then null is returned. 
    const BasicBlock *get_true_dest() const;
    BasicBlock *get_true_dest() {
        return const_cast<BasicBlock*>(
            static_cast<const Brif*>(this)->get_true_dest());
    }

    /// Returns the block this instruction branches to if the condition is false.
    /// If this instruction does not belong to a block, then null is returned.
    const BasicBlock *get_false_dest() const;
    BasicBlock *get_false_dest() {
        return const_cast<BasicBlock*>(
            static_cast<const Brif*>(this)->get_true_dest());
    }

    bool is_terminator() const override { return true; }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A phi node receives values incoming from various predecessor blocks to
/// define a new value based on control flow.
class Phi final : public Instruction {
    friend class Builder;

public:
    using Preds = std::vector<BasicBlock*>;

    /// Represents an edge to a phi node.
    struct CEdge final {
        const Value* value;
        const BasicBlock* pred;
    };

    /// Represents an edge to a phi node.
    struct Edge final {
        Value* value;
        BasicBlock* pred;
    };

private:
    Preds m_preds = {};

    Phi(Type *type, BasicBlock *parent, uint32_t def)
      : Instruction(type, parent, def) {}

public:
    /// Returns the predecessor blocks of this node.
    const Preds &get_preds() const { return m_preds; }
    Preds &get_preds() { return m_preds; }

    /// Returns the |i|-th predecessor block of this node.
    const BasicBlock *get_pred(uint32_t i) const {
        assert(i < num_operands() && "index out of bounds!");
        return m_preds[i];
    }

    BasicBlock *get_pred(uint32_t i) {
        assert(i < num_operands() && "index out of bounds!");
        return m_preds[i];
    }

    /// Returns the number of edges to this node.
    inline uint32_t num_edges() const { return num_operands(); }

    /// Returns the |i|-th edge to this node.
    CEdge get_edge(uint32_t i) const {
        assert(i < num_operands() && "index out of bounds!");
        return CEdge {
            get_operand(i),
            get_pred(i),
        };
    }

    Edge get_edge(uint32_t i) {
        assert(i < num_operands() && "index out of bounds!");
        return Edge {
            get_operand(i),
            get_pred(i),
        };
    }

    /// Adds the given |value| as an incoming edge from the basic block |pred|.
    void add_edge(Value *value, BasicBlock *pred);

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A unop instruction defines a new value by performing an operation on a 
/// lone operand.
class Unop final : public Instruction {
    friend class Builder;

public:
    /// The different kinds of unary operators.
    enum class Op : uint8_t {
        Not, INeg, FNeg,
    };

private:
    uint32_t m_def;
    Op m_op;

    Unop(Type *type, BasicBlock *parent, uint32_t def, Op op, Value *value)
      : Instruction(type, parent, def, { value }), m_op(op) {}

public:
    /// Returns the operator of this instruction.
    Op op() const { return m_op; }

    /// Returns the operand of this instruction.
    const Value *get_value() const { return get_operand(0); }
    Value *get_value() { return get_operand(0); }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A binop isntruction defines a new value by performing an operation on two
/// operands.
class Binop final : public Instruction {
    friend class Builder;

public:
    /// The different kinds of binary operators.
    enum class Op : uint8_t {
        IAdd, ISub, IMul, SDiv, UDiv, SMod, UMod,
        FAdd, FSub, FMul, FDiv,
        And, Or, Xor,
        Shl, Shr, Sar,
    };

private:
    Op m_op;

    Binop(Type *type, BasicBlock *parent, uint32_t def, Op op, Value *lhs, 
          Value *rhs)
      : Instruction(type, parent, def, { lhs, rhs }), m_op(op) {}

public:
    /// Returns the operator of this instruction.
    Op op() const { return m_op; }

    /// Returns the left-side operand of this instruction.
    const Value *get_lhs() const { return get_operand(0); }
    Value *get_lhs() { return get_operand(0); }
    
    /// Returns the right-side operand of this instruction.
    const Value *get_rhs() const { return get_operand(1); }
    Value *get_rhs() { return get_operand(1); }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A cast instruction performs a type cast on a lone operand.
class Cast final : public Instruction {
    friend class Builder;

public:
    /// The different kinds of type casts.
    enum class Kind : uint8_t {
        SExt, ZExt, FExt,
        ITrunc, FTrunc,
        S2F, U2F, F2S, F2U,
        P2I, I2P, Reint,
    };

private:
    Kind m_kind;

    Cast(Type *type, BasicBlock* parent, uint32_t def, Kind kind, Value *value)
      : Instruction(type, parent, def, { value }), m_kind(kind) {}

public:
    /// Returns the kind of type cast this instruction performs.
    Kind kind() const { return m_kind; }

    /// Returns the operand of this type cast.
    const Value *get_value() const { return get_operand(0); }
    Value *get_value() { return get_operand(0); }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

/// A comparison instruction defines a new boolean value based on a predicate
/// between two values.
class Cmp final : public Instruction {
    friend class Builder;

public:
    /// The different kinds of comparison predicates.
    enum class Predicate : uint8_t {
        IEq, FEq,
        INe, FNe,
        Slt, Ult, Flt,
        Sle, Ule, Fle,
        Sgt, Ugt, Fgt,
        Sge, Uge, Fge,
    };

private:
    Predicate m_pred;

    Cmp(Type *type, BasicBlock *parent, uint32_t def, Predicate pred, 
        Value *lhs, Value *rhs)
      : Instruction(type, parent, def, { lhs, rhs }), m_pred(pred) {}

public:
    /// Returns the predicate of this comparison.
    Predicate pred() const { return m_pred; }

    /// Returns the left-side operand of this comparison.
    const Value *get_lhs() const { return get_operand(0); }
    Value *get_lhs() { return get_operand(0); }
    
    /// Returns the right-side operand of this comparison.
    const Value *get_rhs() const { return get_operand(1); }
    Value *get_rhs() { return get_operand(1); }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

} // namespace lir

#endif // LIR_INSTRUCTION_H_
