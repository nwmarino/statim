#ifndef STATIM_BYTECODE_HPP_
#define STATIM_BYTECODE_HPP_

#include "input_file.hpp"
#include "types.hpp"

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace stm {

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
    };

    static Operand get_register(ValueType type, vreg_t reg);
    static Operand get_memory(ValueType type, vreg_t reg, i32 offset);
    static Operand get_stack(ValueType type, i32 offset);
    static Operand get_imm(ValueType type, i64 imm);
    static Operand get_fp(ValueType type, f64 fp);
    static Operand get_string(const char* pString);
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
        Move, Lea, Arg, Copy,
        Branch, Call, Return,
        SExt, ZExt, FExt,
        Trunc, FTrunc,
        Add, Sub, Mul, Div,
        Shl, Sar, Shr,
    };

private:
    Opcode op;
    std::vector<Operand> operands;
    InstDescriptor desc;
    Metadata meta;

    const Instruction* pPrev = nullptr;
    const Instruction* pNext = nullptr;

public:
    Instruction(
        Opcode op, 
        const std::vector<Operand>& operands, 
        const InstDescriptor& desc, 
        const Metadata& meta);

    ~Instruction() = default;

    Opcode get_opcode() const { return op; }

    const std::vector<Operand>& get_operands() const { return operands; }

    u32 num_operands() const { return operands.size(); }

    InstDescriptor& get_desc() { return desc; }

    const InstDescriptor& get_desc() const { return desc; }

    const Metadata& get_metadata() const { return meta; }

    const Instruction* get_prev() const { return pPrev; }

    const Instruction* get_next() const { return pNext; }

    void set_prev(const Instruction* pInst) { pPrev = pInst; }

    void set_next(const Instruction* pInst) { pNext = pInst; }

    bool is_terminator() const;
};

class BasicBlock final {
    u32 id;
    
    const Function* pParent;

    Instruction* pFront = nullptr;
    Instruction* pBack = nullptr;

    const BasicBlock* pPrev = nullptr;
    const BasicBlock* pNext = nullptr;

public:
    BasicBlock(Function* pParent = nullptr);

    u32 get_id() const { return id; }

    const Function* get_parent() const { return pParent; }

    Instruction* front() const { return pFront; }

    Instruction* back() const { return pBack; }

    const BasicBlock* prev() const { return pPrev; }

    const BasicBlock* next() const { return pNext; }

    void set_prev(const BasicBlock* pBlock) { pPrev = pBlock; }

    void set_next(const BasicBlock* pBlock) { pNext = pBlock; }

    bool empty() const { return pFront == nullptr; }

    bool terminates() const;

    u32 terminators() const;

    void prepend(Instruction* pInst);

    void append(Instruction* pInst);
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

    const std::string& get_name() const { return name; }

    const std::vector<ValueType>& get_args() const { return args; }

    ValueType get_arg(u32 idx) const { return args.at(idx); }

    ValueType get_return() const { return ret; }

    BasicBlock* front() const { return pFront; }

    BasicBlock* back() const { return pBack; }

    bool empty() const { return pFront == nullptr; }

    void prepend(BasicBlock* pBlock);

    void append(BasicBlock* pBlock);
};

class Frame final {
    InputFile& file;
    std::map<std::string, Function*> functions;

public:
    Frame(InputFile& file) : file(file), functions() {};

    std::vector<Function*> get_functions() const {
        std::vector<Function*> fns { functions.size() };
        for (auto fn : functions)
            fns.push_back(fn.second);

        return fns;
    }

    u32 num_functions() const { return functions.size(); }
};

} // namespace stm

#endif // STATIM_BYTECODE_HPP_
