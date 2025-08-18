#ifndef STATIM_BYTECODE_HPP_
#define STATIM_BYTECODE_HPP_

#include "input_file.hpp"
#include "types.hpp"

#include <map>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace stm {

class BasicBlock;
class Function;

using vreg_t = u32;

struct Metadata final {
    InputFile& file;
    u32        line;  
};

enum class ValueType : u8 {
    None,
    Int8, Int16, Int32, Int64,
    Float32, Float64,
    Pointer,
};

struct Operand final {
    enum class Kind : u8 {
        Register, // v0
        Memory, // (v0 + 8)
        Stack, // stack + x
        Integer, // 0
        Float, // 3.14
        String, // "Hey!"
        Block, // #1
        Function, // @foo
    } kind;

    ValueType type;    

    union {
        vreg_t reg;

        struct {
            vreg_t reg;
            i32 offset;
        } memory;

        struct {
            i32 offset;
        } stack;

        i64 imm;
        
        f64 fp;

        const char* pString;

        BasicBlock* pBlock;

        Function* pFunction;
    };

    static Operand get_register(ValueType type, vreg_t reg);
    static Operand get_memory(ValueType type, vreg_t reg, i32 offset);
    static Operand get_stack(ValueType type, i32 offset);
    static Operand get_imm(ValueType type, i64 imm);
    static Operand get_fp(ValueType type, f64 fp);
    static Operand get_string(const char* pString);
    static Operand get_block(BasicBlock* pBlock);
    static Operand get_function(Function* pFunction);

    void print(std::ostream& os) const;
};

struct InstDescriptor final {
    /// Bytes read during a memory access.
    u32 access;

    InstDescriptor() = default;
};

class Instruction final {
public:
    enum class Opcode : u8 {
        Constant, String,
        Move, Lea, Copy,
        Load_Arg,
        Store_Arg,
        Branch, BranchIf, Call, Return,
        SExt, ZExt, FExt,
        Trunc, FTrunc,
        Add, Sub, Mul, Div,
        Shl, Sar, Shr,
    };

private:
    u64 position;
    Opcode op;
    std::vector<Operand> operands;
    InstDescriptor desc;
    Metadata meta;

    BasicBlock* pParent = nullptr;

    Instruction* pPrev = nullptr;
    Instruction* pNext = nullptr;

public:
    Instruction(
        u64 position,
        Opcode op, 
        const std::vector<Operand>& operands, 
        const InstDescriptor& desc, 
        const Metadata& meta,
        BasicBlock* pParent = nullptr);

    ~Instruction() = default;

    u64 get_position() const { return position; }

    Opcode get_opcode() const { return op; }

    const std::vector<Operand>& get_operands() const { return operands; }

    u32 num_operands() const { return operands.size(); }

    InstDescriptor& get_desc() { return desc; }

    const InstDescriptor& get_desc() const { return desc; }

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

    std::vector<BasicBlock*> preds {};
    std::vector<BasicBlock*> succs {};
    std::vector<vreg_t> defs {};

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

    const std::vector<BasicBlock*>& predecessors() const { return preds; }

    const std::vector<BasicBlock*>& successors() const { return succs; }

    const std::vector<vreg_t>& definitions() const { return defs; }

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

struct Register final {
    vreg_t num;
};

class Function final {
    std::unordered_map<vreg_t, Register> regs;

    std::string name;
    std::vector<ValueType> args;
    ValueType ret;

    BasicBlock* pFront = nullptr;
    BasicBlock* pBack = nullptr;

public:
    Function(
        const std::string& name, 
        const std::vector<ValueType>& args, 
        ValueType ret);

    ~Function();

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
