#ifndef STATIM_BYTECODE_HPP_
#define STATIM_BYTECODE_HPP_

#include "input_file.hpp"
#include "source_loc.hpp"
#include "types.hpp"

#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace stm {

class BasicBlock;
class StackSlot;
class Function;

struct Metadata final {
    InputFile& file;
    u32        line;  

    Metadata(InputFile& file, u32 line) : file(file), line(line) {};

    Metadata(const Span& span) : file(span.begin.file), line(span.begin.line) {};

    Metadata(const SourceLocation& loc) : file(loc.file), line(loc.line) {};
};

struct Register final {
    u32 id;
};

struct Immediate final {
    enum class Kind : u8 {
        Integer, 
        Float, 
        String,
    } kind;

    union {
        i64 i;
        f64 f;
        const char* s;
    };

    Immediate(i64 i) : kind(Kind::Integer), i(i) {};
    Immediate(f64 f) : kind(Kind::Float), f(f) {};
    Immediate(const char* s) : kind(Kind::String), s(s) {};
};

struct MemoryRef final {
    Register base;
    i32 offset;
};

struct ArgumentRef final {
    u32 index;
};

struct BlockRef final {
    BasicBlock* pBlock;
};

struct FunctionRef final {
    Function* pFunction;
};

struct Operand final {
    enum class Kind : u8 {
        Register,
        Immediate,
        Memory,
        Argument,
        Block,
        Function,
    } kind;

    union {
        Register reg;
        Immediate imm;
        MemoryRef mem;
        ArgumentRef arg;
        BlockRef block;
        FunctionRef function;
    };

    Operand(Register reg);
    Operand(Immediate imm);
    Operand(MemoryRef mem);
    Operand(ArgumentRef arg);
    Operand(BlockRef block);
    Operand(FunctionRef function);

    void print(std::ostream& os) const;
};

/// Different types of instruction opcodes.
enum class Opcode : u8 {
    Constant,
    Move,
    Lea, 
    Copy,
    Jump, 
    BranchTrue, BranchFalse,
    Return,
    Call, 
    Add, Sub, Mul, Div,
    Inc, Dec,
    Neg,
    And, Or, Xor, Not,
    Shl, Sar, Shr,
    SExt, ZExt, FExt,
    Trunc, FTrunc,
    SI2SS, SI2SD,
    UI2SS, UI2SD,
    SS2SI, SD2SI,
    SS2UI, SD2UI,
    Cmpeq, Cmpne, 
    Cmpoeq, Cmpone, 
    Cmpuneq, Cmpunne,
    Cmpslt, Cmpsle, Cmpsgt, Cmpsge,
    Cmpult, Cmpule, Cmpugt, Cmpuge,
    Cmpolt, Cmpole, Cmpogt, Cmpoge,
    Cmpunlt, Cmpunle, Cmpungt, Cmpunge,
};

class Instruction final {
public:
    enum class Size : u8 {
        None,
        Byte, // b
        Half, // h
        Quad, // q
        Word, // w
        Single, // ss
        Double, // sd
    };

private:
    u32 m_position;
    Opcode m_op;
    std::vector<Operand> m_operands;
    Metadata m_meta;
    Size m_size;
    std::string m_comment;
    BasicBlock* m_parent = nullptr;
    Instruction* m_prev = nullptr;
    Instruction* m_next = nullptr;

    Instruction(
        u32 position,
        Opcode op,
        const std::vector<Operand>& operands, 
        const Metadata& meta,
        BasicBlock* insert,
        Size size,
        const std::string& comment);

public:
    static void create(
        BasicBlock* block,
        Opcode op, 
        const std::vector<Operand>& operands, 
        const Metadata& meta,
        Size size = Size::None,
        const std::string& comment = "");

    ~Instruction() = default;

    /// \returns The position of this instruction in its parent frame.
    u32 position() const { return m_position; }

    /// \returns The opcode of this instruction.
    Opcode opcode() const { return m_op; }

    /// \returns The operands of thins instruction.
    const std::vector<Operand>& operands() const { return m_operands; }

    /// \returns The number of operands in this instruction.
    u32 num_operands() const { return m_operands.size(); }

    /// \returns The metadata of this instruction.
    const Metadata& metadata() const { return m_meta; }

    /// \returns The instruction previous to this one, if it exists.
    Instruction* prev() const { return m_prev; }

    /// \returns The instruction after this one, if it exists.
    Instruction* next() const { return m_next; }

    /// Set the instruction before this one to \p inst.
    void set_prev(Instruction* inst) { m_prev = inst; }

    /// Set the instruction after this one to \p inst.
    void set_next(Instruction* inst) { m_next = inst; }

    /// \returns The parent block of this instruction.
    BasicBlock* parent() const { return m_parent; }

    /// \returns The size of this instruction.
    Size size() const { return m_size; }

    /// \returns `true` if this instruction is a terminator.
    bool is_terminator() const;

    /// \returns `true` if this instruction is a comparison (affecting flags).
    bool is_comparison() const;

    void print(std::ostream& os) const;
};

class BasicBlock final {
    const Function* pParent;

    Instruction* pFront = nullptr;
    Instruction* pBack = nullptr;

    BasicBlock* pPrev = nullptr;
    BasicBlock* pNext = nullptr;

public:
    BasicBlock(Function* pParent = nullptr);

    ~BasicBlock();

    const Function* get_parent() const { return pParent; }

    void set_parent(Function* pFunction) { pParent = pFunction; }

    Instruction* front() const { return pFront; }

    Instruction* back() const { return pBack; }

    BasicBlock* prev() const { return pPrev; }

    BasicBlock* next() const { return pNext; }

    void set_prev(BasicBlock* pBlock) { pPrev = pBlock; }

    void set_next(BasicBlock* pBlock) { pNext = pBlock; }

    /// Test if this basic block is empty (has no instructions).
    bool empty() const { return pFront == nullptr; }

    /// Get the size of this basic block in instructions.
    u32 size() const;

    /// Get the number of this basic block, that is, it's position in the
    /// parent function.
    u32 get_number() const;

    /// Test if this basic block terminates at any point.
    bool terminates() const;

    /// Get the number of terminating instructions in this block.
    u32 terminators() const;

    /// Prepend an instruction to beginning of this block.
    void prepend(Instruction* pInst);

    /// Append an instruction to the end of this block.
    void append(Instruction* pInst);

    void print(std::ostream& os) const;
};

class StackSlot final {
    const Function* pParent;

    std::string name;
    u32 offset;

public:
    StackSlot(
        const std::string& name, 
        u32 offset, 
        Function* pParent = nullptr);
    
    const std::string& get_name() const { return name; }

    u32 get_offset() const { return offset; }
};

class Function final {
    std::vector<StackSlot*> stack {};
    std::string name;
    BasicBlock* pFront = nullptr;
    BasicBlock* pBack = nullptr;

public:
    Function(const std::string& name);

    ~Function();

    const std::vector<StackSlot*>& get_stack() const { return stack; }

    StackSlot* get_slot(const std::string& name);

    void add_slot(StackSlot* pSlot) { stack.push_back(pSlot); }

    u32 get_stack_size() const;

    const std::string& get_name() const { return name; }

    BasicBlock* front() const { return pFront; }

    BasicBlock* back() const { return pBack; }

    bool empty() const { return pFront == nullptr; }

    u32 num_blocks() const;

    void prepend(BasicBlock* pBlock);

    void append(BasicBlock* pBlock);

    void print(std::ostream& os) const;
};

class Frame final {
    InputFile& file;
    std::map<std::string, Function*> functions;

public:
    Frame(InputFile& file) : file(file), functions() {};

    ~Frame();

    Function* get_function(const std::string& name) const {
        return functions.at(name);
    }

    std::vector<Function*> get_functions() const {
        std::vector<Function*> fns { functions.size() };
        for (auto fn : functions)
            fns.push_back(fn.second);

        return fns;
    }

    u32 num_functions() const { return functions.size(); }

    void add_function(Function* F, const std::string& name = "") { 
        functions.emplace(name.empty() ? F->get_name() : name, F); 
    }

    void print(std::ostream& os) const;
};

} // namespace stm

#endif // STATIM_BYTECODE_HPP_
