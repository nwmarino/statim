#ifndef STATIM_BYTECODE_HPP_
#define STATIM_BYTECODE_HPP_

#include "input_file.hpp"
#include "source_loc.hpp"
#include "types.hpp"

#include <map>
#include <optional>
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

    Metadata(const Span& span) : file(span.begin.file), line(span.begin.line) {};

    Metadata(const SourceLocation& loc) : file(loc.file), line(loc.line) {};
};

enum class ValueType : u8 {
    None,
    Int8, Int16, Int32, Int64,
    Float32, Float64,
    Pointer,
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

struct StackRef final {
    i32 offset;
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
        MemoryRef,
        StackRef,
        BlockRef,
        FunctionRef,
    } kind;

    union {
        Register reg;
        Immediate imm;
        MemoryRef mem;
        StackRef stack;
        BlockRef block;
        FunctionRef function;
    };

    Operand(Register reg);
    Operand(Immediate imm);
    Operand(MemoryRef mem);
    Operand(StackRef stack);
    Operand(BlockRef block);
    Operand(FunctionRef function);

    void print(std::ostream& os) const;
};

struct InstructionDesc final {
    std::optional<u32> size = std::nullopt;
};

class Instruction final {
    static u32 s_position;

public:
    enum class Opcode : u8 {
        Constant, Float, String,
        Move, Lea, Copy,
        Load_Arg,
        Store_Arg,
        Jump, Branch, Call, Return,
        SExt, ZExt, FExt,
        Trunc, FTrunc,
        Add, Sub, Mul, Div,
        Shl, Sar, Shr,
    };

private:
    u64 pos;
    Opcode op;
    std::vector<Operand> operands;
    InstructionDesc desc;
    Metadata meta;

    BasicBlock* pParent = nullptr;

    Instruction* pPrev = nullptr;
    Instruction* pNext = nullptr;

public:
    Instruction(
        Opcode op, 
        const std::vector<Operand>& operands, 
        const Metadata& meta,
        const InstructionDesc& desc = {},
        BasicBlock* pParent = nullptr);

    ~Instruction() = default;

    u32 get_position() const { return pos; }

    Opcode get_opcode() const { return op; }

    const std::vector<Operand>& get_operands() const { return operands; }

    u32 num_operands() const { return operands.size(); }

    const Metadata& get_metadata() const { return meta; }

    Instruction* prev() const { return pPrev; }

    Instruction* next() const { return pNext; }

    void set_prev(Instruction* pInst) { pPrev = pInst; }

    void set_next(Instruction* pInst) { pNext = pInst; }

    bool is_terminator() const;

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
    std::map<std::string, StackSlot*>       stack {};
    std::string                             name;
    std::vector<ValueType>                  args;
    ValueType                               ret;
    BasicBlock*                             pFront = nullptr;
    BasicBlock*                             pBack = nullptr;

public:
    Function(
        const std::string& name, 
        const std::vector<ValueType>& args, 
        ValueType ret);

    ~Function();

    const std::map<std::string, StackSlot*>& get_stack() const { return stack; }

    StackSlot* get_slot(const std::string& name) { return stack.at(name); }

    void add_slot(StackSlot* pSlot) { stack.emplace(pSlot->get_name(), pSlot); }

    u32 get_stack_size() const;

    const std::string& get_name() const { return name; }

    const std::vector<ValueType>& get_args() const { return args; }

    ValueType get_arg(u32 idx) const { return args.at(idx); }

    ValueType get_return() const { return ret; }

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
