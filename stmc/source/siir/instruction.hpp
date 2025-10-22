#ifndef STATIM_SIIR_INSTRUCTION_HPP_
#define STATIM_SIIR_INSTRUCTION_HPP_

#include "siir/constant.hpp"
#include "siir/user.hpp"
#include "siir/value.hpp"

namespace stm {
namespace siir {

class BasicBlock;

/// An operand to a PHI node, wrapping over a value and the basic block it
/// comes from.
class PhiOperand final : public Value {
    /// The value in this edge.
    Value* m_value;

    /// The parent predecessor block that this operand's value comes from.
    BasicBlock* m_pred;

public:
    PhiOperand(Value* value, BasicBlock* pred);

    PhiOperand(const PhiOperand&) = delete;
    PhiOperand& operator = (const PhiOperand&) = delete;

    operator Value*() const { return m_value; }
    operator BasicBlock*() const { return m_pred; }

    /// Returns the value of this incoming phi edge.
    const Value* get_value() const { return m_value; }
    Value* get_value() { return m_value; }

    /// Returns the predecessor basic block of this incoming phi edge.
    const BasicBlock* get_pred() const { return m_pred; }
    BasicBlock* get_pred() { return m_pred; }

    void print(std::ostream& os) const override;
};

/// Potential opcodes for an IR instruction.
enum Opcode : u16 {
    INST_OP_NOP,
    INST_OP_CONSTANT,
    INST_OP_STRING,
    INST_OP_LOAD,
    INST_OP_STORE,
    INST_OP_ACCESS_PTR,
    INST_OP_SELECT,
    INST_OP_BRANCH_IF,
    INST_OP_JUMP,
    INST_OP_PHI,
    INST_OP_RETURN,
    INST_OP_ABORT,
    INST_OP_UNREACHABLE,
    INST_OP_CALL,
    INST_OP_IADD,
    INST_OP_FADD,
    INST_OP_ISUB,
    INST_OP_FSUB,
    INST_OP_SMUL,
    INST_OP_UMUL,
    INST_OP_FMUL,
    INST_OP_SDIV,
    INST_OP_UDIV,
    INST_OP_FDIV,
    INST_OP_SREM,
    INST_OP_UREM,
    INST_OP_AND,
    INST_OP_OR,
    INST_OP_XOR,
    INST_OP_SHL,
    INST_OP_SHR,
    INST_OP_SAR,
    INST_OP_NOT,
    INST_OP_INEG,
    INST_OP_FNEG,
    INST_OP_SEXT,
    INST_OP_ZEXT,
    INST_OP_FEXT,
    INST_OP_ITRUNC,
    INST_OP_FTRUNC,
    INST_OP_SI2FP,
    INST_OP_UI2FP,
    INST_OP_FP2SI,
    INST_OP_FP2UI,
    INST_OP_P2I,
    INST_OP_I2P,
    INST_OP_REINTERPET,
    INST_OP_CMP_IEQ,
    INST_OP_CMP_INE,
    INST_OP_CMP_OEQ,
    INST_OP_CMP_ONE,
    INST_OP_CMP_UNEQ,
    INST_OP_CMP_UNNE,
    INST_OP_CMP_SLT,
    INST_OP_CMP_SLE,
    INST_OP_CMP_SGT,
    INST_OP_CMP_SGE,
    INST_OP_CMP_ULT,
    INST_OP_CMP_ULE,
    INST_OP_CMP_UGT,
    INST_OP_CMP_UGE,
    INST_OP_CMP_OLT,
    INST_OP_CMP_OLE,
    INST_OP_CMP_OGT,
    INST_OP_CMP_OGE,
    INST_OP_CMP_UNLT,
    INST_OP_CMP_UNLE,
    INST_OP_CMP_UNGT,
    INST_OP_CMP_UNGE,
};

/// Returns the string equivelant of |op|.
std::string opcode_to_string(Opcode op);

/// An instruction that potentially defines a value.
class Instruction final : public User {
    friend class InstBuilder;
    
    using OperandList = std::vector<Value*>;
    using iterator = OperandList::iterator;
    using const_iterator = OperandList::const_iterator;

    /// The resulting id of this instruction. This is the value on the left
    /// hand side of a dump, i.e. `v2` in `v2 = IADD ...`. A sentinel value of 
    /// 0 here reserves that the instruction does not define a value, i.e. a
    /// store or return instruction.
    u32 m_result;

    /// The opcode of this instruction. Changing this after instruction
    /// creation may invalidate the instruction due to how operands are
    /// interpreted.
    Opcode m_opcode;

    /// Optional data field used for some instructions. For example, load/store
    /// instructions use this field for value alignment details.
    u16 m_data = 0;

    /// The basic block that this instruction is contained in.
    BasicBlock* m_parent;

    /// Links to the previous and next instructions in the parent basic block.
    /// These pointers functionally make up the doubly-linked list managed
    /// by the parent BasicBlock.
    Instruction* m_prev = nullptr;
    Instruction* m_next = nullptr;

    /// Create a new non-defining instruction.
    ///
    /// Private constructor to be used by the InstBuilder class.
    Instruction(Opcode opcode, BasicBlock* parent, 
                const std::vector<Value*>& operands = {});
    
    /// Create a new defining instruction.
    ///
    /// Private construtor to be used by the InstBuilder class.
    Instruction(u32 result, const Type* type, Opcode opcode, BasicBlock* parent,
                const std::vector<Value*>& operands = {});

public:
    Instruction(const Instruction&) = delete;
    Instruction& operator = (const Instruction&) = delete;

    ~Instruction() = default;

    /// Returns the opcode of this instruction.
    Opcode opcode() const { return m_opcode; }

    /// Returns the id defined by this instruction, and 0 if it does not define
    /// a value.
    u32 result_id() const { return m_result; }

    /// Returns true if this instruction defines a value.
    bool is_def() const { return m_result != 0; }

    /// Returns the value operand at position |i|. Fails if the provided index
    /// is out of bounds of the operand list.
    const Value* get_operand(u32 i) const;
    Value* get_operand(u32 i) {
        return const_cast<Value*>(
            static_cast<const Instruction*>(this)->get_operand(i));
    }

    /// Returns any data associated with this instruction.
    u16 get_data() const { return m_data; }

    /// Returns a reference to the data field of this instruction.
    u16& data() { return m_data; }

    /// Returns the parent block of this instruction.
    const BasicBlock* get_parent() const { return m_parent; }
    BasicBlock* get_parent() { return m_parent; }

    /// Mutate the parent basic block of this instruction to |blk|.
    void set_parent(BasicBlock* blk) { m_parent = blk; }
 
    /// Clears the parent basic block of this instruction. Does not actually 
    /// remove this instruction from the old block.
    void clear_parent() { m_parent = nullptr; }

    /// Prepends this instruction to |blk|. Assumes that this instruction is 
    /// unlinked and free-floating.
    void prepend_to(BasicBlock* blk);

    /// Append this instruction to |blk|. Assumes that this instruction is 
    /// unlinked and free-floating.
    void append_to(BasicBlock* blk);

    /// Insert this instruction into the position immediately before |inst|.
    void insert_before(Instruction* inst);

    /// Insert this instruction into the position immediately after |inst|.
    void insert_after(Instruction* inst);

    /// Returns the instruction previous to this one in the parent basic block.
    const Instruction* prev() const { return m_prev; }
    Instruction* prev() { return m_prev; }

    /// Returns instruction after this one in the parent basic block.
    const Instruction* next() const { return m_next; }
    Instruction* next() { return m_next; }

    void set_prev(Instruction* inst) { m_prev = inst; }
    void set_next(Instruction* inst) { m_next = inst; }

    /// Detach this instruction from its parent block. Does not destroy the
    /// instruction.
    void detach_from_parent();

    /// Returns true if this instruction does nothing.
    bool is_nop() const { return opcode() == INST_OP_NOP; }

    /// Returns true if this instruction defines a constant value.
    bool is_const() const { return opcode() == INST_OP_CONSTANT; }

    /// Returns true if this instruction loads a value from memory.
    bool is_load() const { return opcode() == INST_OP_LOAD; }

    /// Returns true if this instruction stores a value to memory.
    bool is_store() const { return opcode() == INST_OP_STORE; }

    /// Returns true if this instruction jumps to another basic block.
    bool is_jump() const { return opcode() == INST_OP_JUMP; }

    /// Returns true if this instruction branches conditionally.
    bool is_branch_if() const { return opcode() == INST_OP_BRANCH_IF; }

    /// Returns true if this instruction is a phi node.
    bool is_phi() const { return opcode() == INST_OP_PHI; }

    /// Returns true if this instruction aborts execution.
    bool is_abort() const { return opcode() == INST_OP_ABORT; }

    /// Returns true if this instruction returns from a function call.
    bool is_return() const { return opcode() == INST_OP_RETURN; }

    /// Returns true if this instruction aborts execution or returns from a
    /// function call.
    bool is_abort_or_return() const { return is_abort() | is_return(); }

    /// Returns true if this instruction marks a function call.
    bool is_call() const { return opcode() == INST_OP_CALL; }

    /// Returns true if this instruction terminates a basic block.
    bool is_terminator() const;

    /// Returns true if this instruction performs a comparison between two
    /// operands.
    bool is_comparison() const;

    /// Returns true if this is an ordered floating point comparison.
    bool is_ordered_cmp() const;

    /// Returns true if this is an unordered floating point comparison.
    bool is_unordered_cmp() const;

    /// Returns true if this instructions performs some form of type cast.
    /// This includes pointer casts like pointer to integer, and reinterprets.
    bool is_cast() const;

    /// Returns true if this instruction deals with floating point values only.
    /// This generally only works for things like comparisons and arithmetic 
    /// but not generic load/store/constant/etc. instructions.
    bool operates_on_floats() const;

    /// Add a new incoming value edge to a PHI node. This only works for
    /// instructions with the PHI opcode.
    void add_incoming(CFG& cfg, Value* value, BasicBlock* pred);

    /// Returns true if this instruction can be considered "dead" in a trivial
    /// manner. This includes defining instructions that are considered
    /// non-volatile, non-branching instructions, and unused calls to functions
    /// without side effects.
    bool is_trivially_dead() const;

    void print(std::ostream& os) const override;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_INSTRUCTION_HPP_
