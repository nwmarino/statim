#ifndef STATIM_SIIR_INSTBUILDER_HPP_
#define STATIM_SIIR_INSTBUILDER_HPP_

#include "siir/basicblock.hpp"
#include "siir/constant.hpp"
#include "siir/instruction.hpp"
#include "siir/type.hpp"

#include <cassert>

namespace stm {
namespace siir {

/// A system of convenience to build instructions.
class InstBuilder final {
public:
    enum InsertMode : u8 {
        Prepend, Append,
    };

private:
    /// The parent graph, used for type and constant pooling.
    CFG& m_cfg;

    /// The current basic block insertion point.
    BasicBlock* m_insert = nullptr;

    InsertMode m_mode = Append;

public:
    /// Create a new instruction builder for graph |cfg|.
    InstBuilder(CFG& cfg) : m_cfg(cfg) {}

    ~InstBuilder() = default;

    /// Returns the basic block that the builder is currently inserting new 
    /// instructions into.
    const BasicBlock* get_insert() const { return m_insert; }
    BasicBlock* get_insert() { return m_insert; }

    /// Set the builder insertion point to |blk|.
    void set_insert(BasicBlock* blk) { m_insert = blk; }

    /// Clear the current insertion point of the builder.
    void clear_insert() { m_insert = nullptr; }

    /// Returns the mode of insertion this builder is in.
    InsertMode get_mode() const { return m_mode; }

    /// Set the insertion mode of this builder to |mode|.
    void set_insert_mode(InsertMode mode) { m_mode = mode; }

    /// Insert |inst| to the current insertion point, if it is set.
    void insert(Instruction* inst);

    /// Create and insert a new instruction with opcode |op|, possible result
    /// id |result|, and operand list |operands|. Returns the newly created
    /// instruction.
    Instruction* insert(Opcode op, u32 result = 0, const Type* type = nullptr,
                        const std::vector<Value*>& operands = {});

    /// Create a new no operation instruction.
    Instruction* build_nop();

    /// Create a new const instruction defining |constant| as a value.
    Instruction* build_const(Constant* constant);

    /// Create a new constant string instruction defining |string| as a value.
    Instruction* build_string(ConstantString* string);

    /// Create a new load instruction that loads a value typed with |type| from
    /// source pointer |src|. The natural alignment of |type| will be used.
    Instruction* build_load(const Type* type, Value* src);

    /// Create a new load instruction that loads a value typed with |type| from
    /// source pointer |src| with the desired alignment |align|.
    Instruction* build_aligned_load(const Type* type, Value* src, u16 align);

    /// Create a new store instruction that stores |value| to |dst|.
    /// The natural alignment of |value| will be used.
    Instruction* build_store(Value* value, Value* dst);

    /// Create a new store instruction that stores |value| to |dst| with the
    /// desired alignment |align|.
    Instruction* build_aligned_store(Value* value, Value* dst, u16 align);

    /// Create a new pointer access instruction that access |src| at element
    /// offset |idx|. The |type| argument indicates the resulting pointer.
    Instruction* build_ap(const Type* type, Value* src, Value* idx);

    /// Create a new selection instruction that chooses |tvalue| or |fvalue| 
    /// based on the result of |cond|.
    Instruction* build_select(Value* cond, Value* tvalue, Value* fvalue);

    /// Create a new conditional branching instruction that chooses |tdst| or
    /// |fdst| based on the result of |cond|.
    Instruction* build_brif(Value* cond, BasicBlock* tdst, BasicBlock* fdst);

    /// Create a new jump instruction that branches to |dst|.
    Instruction* build_jmp(BasicBlock* dst);

    /// Create a new phi node of the given type.
    Instruction* build_phi(const Type* type);

    /// Create a new return instruction that returns |value|.
    Instruction* build_ret(Value* value);

    /// Create a new return instruction that returns nothing.
    Instruction* build_ret_void() { return build_ret(nullptr); }

    /// Create a new abort instruction that stops execution.
    Instruction* build_abort();

    /// Create a new unreachable instruction, used as a pseudo-terminator to
    /// mark places that should be unreachable by control flow.
    Instruction* build_unreachable();

    /// Create a new call instruction to |callee| with argument list |args|.
    Instruction* build_call(const FunctionType* type, Value* callee, 
                            const std::vector<Value*>& args);

    /// Create a new integer equality comparison.
    Instruction* build_cmp_ieq(Value* lhs, Value* rhs);

    /// Create a new integer inequality comparison.
    Instruction* build_cmp_ine(Value* lhs, Value* rhs);

    /// Create a new floating point ordered equality comparison.
    Instruction* build_cmp_oeq(Value* lhs, Value* rhs);

    /// Create a new floating point ordered inequality comparison.
    Instruction* build_cmp_one(Value* lhs, Value* rhs);

    /// Create a new floating point unordered equality comparison.
    Instruction* build_cmp_uneq(Value* lhs, Value* rhs);

    /// Create a new floating point unordered inequality comparison.
    Instruction* build_cmp_unne(Value* lhs, Value* rhs);

    /// Create a new signed integer less than comparison.
    Instruction* build_cmp_slt(Value* lhs, Value* rhs);

    /// Create a new signed integer less than equals comparison.
    Instruction* build_cmp_sle(Value* lhs, Value* rhs);

    /// Create a new signed integer greater than comparison.
    Instruction* build_cmp_sgt(Value* lhs, Value* rhs);

    /// Create a new signed integer greater than equals comparison.
    Instruction* build_cmp_sge(Value* lhs, Value* rhs);

    /// Create a new unsigned integer less than comparison.
    Instruction* build_cmp_ult(Value* lhs, Value* rhs);

    /// Create a new unsigned integer less than equals comparison.
    Instruction* build_cmp_ule(Value* lhs, Value* rhs);

    /// Create a new unsigned integer greater than comparison.
    Instruction* build_cmp_ugt(Value* lhs, Value* rhs);

    /// Create a new unsigned integer greater than equals comparison.
    Instruction* build_cmp_uge(Value* lhs, Value* rhs);

    /// Create a new floating point ordered less than comparison.
    Instruction* build_cmp_olt(Value* lhs, Value* rhs);

    /// Create a new floating point ordered less than equals comparison.
    Instruction* build_cmp_ole(Value* lhs, Value* rhs);

    /// Create a new floating point ordered greater than comparison.
    Instruction* build_cmp_ogt(Value* lhs, Value* rhs);

    /// Create a new floating point ordered greater than equals comparison.
    Instruction* build_cmp_oge(Value* lhs, Value* rhs);

    /// Create a new floating point unordered less than comparison.
    Instruction* build_cmp_unlt(Value* lhs, Value* rhs);

    /// Create a new floating point unordered less than equals comparison.
    Instruction* build_cmp_unle(Value* lhs, Value* rhs);

    /// Create a new floating point unordered greater than comparison.
    Instruction* build_cmp_ungt(Value* lhs, Value* rhs);

    /// Create a new floating point unordered greater than equals comparison.
    Instruction* build_cmp_unge(Value* lhs, Value* rhs);

    /// Create a new integer addition instruction.
    Instruction* build_iadd(Value* lhs, Value* rhs);

    /// Create a new floating point addition instruction.
    Instruction* build_fadd(Value* lhs, Value* rhs);

    /// Create a new integer subtraction instruction.
    Instruction* build_isub(Value* lhs, Value* rhs);

    /// Create a new floating point subtraction instruction.
    Instruction* build_fsub(Value* lhs, Value* rhs);

    /// Create a new signed integer multiplication instruction.
    Instruction* build_smul(Value* lhs, Value* rhs);

    /// Create a new unsigned integer multiplication instruction.
    Instruction* build_umul(Value* lhs, Value* rhs);

    /// Create a new floating point multiplication instruction.
    Instruction* build_fmul(Value* lhs, Value* rhs);

    /// Create a new signed integer division instruction.
    Instruction* build_sdiv(Value* lhs, Value* rhs);

    /// Create a new unsigned integer division instruction.
    Instruction* build_udiv(Value* lhs, Value* rhs);

    /// Create a new floating point division instruction.
    Instruction* build_fdiv(Value* lhs, Value* rhs);

    /// Create a new signed integer remainder instruction.
    Instruction* build_srem(Value* lhs, Value* rhs);

    /// Create a new unsigned integer remainder instruction.
    Instruction* build_urem(Value* lhs, Value* rhs);

    /// Create a new bitwise and instruction.
    Instruction* build_and(Value* lhs, Value* rhs);

    /// Create a new bitwise or instruction.
    Instruction* build_or(Value* lhs, Value* rhs);

    /// Create a new bitwise xor instruction.
    Instruction* build_xor(Value* lhs, Value* rhs);

    /// Create a new bitwise logical left shift instruction.
    Instruction* build_shl(Value* lhs, Value* rhs);

    /// Create a new bitwise logical right shift instruction.
    Instruction* build_shr(Value* lhs, Value* rhs);

    /// Create a new bitwise arithmetic right shift instruction.
    Instruction* build_sar(Value* lhs, Value* rhs);

    /// Create a new bitwise not instruction.
    Instruction* build_not(Value* value);

    /// Create a new integer negate instruction.
    Instruction* build_ineg(Value* value);

    /// Create a new floating point negate instruction.
    Instruction* build_fneg(Value* value);

    /// Create a new integer sign extension instruction. The instruction will
    /// extend |value| to the provided type.
    Instruction* build_sext(const Type* type, Value* value);

    /// Create a new integer zero extension instruction. The instruction will
    /// extend |value| to the provided type.
    Instruction* build_zext(const Type* type, Value* value);

    /// Create a new floating point extension instruction. The instruction will
    /// extend |value| to the provided type.
    Instruction* build_fext(const Type* type, Value* value);

    /// Create a new integer truncation instruction. The instruction will
    /// truncate |value| to the provided type.
    Instruction* build_itrunc(const Type* type, Value* value);

    /// Create a new floating point truncation instruction. The instruction 
    /// will truncate |value| to the provided type.
    Instruction* build_ftrunc(const Type* type, Value* value);

    /// Create a new signed integer to floating point conversion instruction.
    Instruction* build_si2fp(const Type* type, Value* value);

    /// Create a new unsigned integer to floating point conversion instruction.
    Instruction* build_ui2fp(const Type* type, Value* value);

    /// Create a new floating point to signed integer conversion instruction.
    Instruction* build_fp2si(const Type* type, Value* value);

    /// Create a new floating point to unsigned integer conversion instruction.
    Instruction* build_fp2ui(const Type* type, Value* value);
    
    /// Create a new pointer to integer conversion instruction. The instruction
    /// will convert |value| to the provided type.
    Instruction* build_p2i(const Type* type, Value* value);

    /// Create a new integer to pointer conversion instruction. The instruction
    /// will convert |value| to the provided type.
    Instruction* build_i2p(const Type* type, Value* value);

    /// Create a new reinterpretation cast instruction. The instruction will 
    /// convert |value| to the provided type.
    Instruction* build_reint(const Type* type, Value* value);
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_INSTBUILDER_HPP_
