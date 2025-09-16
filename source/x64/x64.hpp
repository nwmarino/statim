#ifndef STATIM_SIIR_X64_H_
#define STATIM_SIIR_X64_H_

#include "siir/instruction.hpp"
#include "siir/machine_register.hpp"
#include "siir/machine_object.hpp"
#include "types/types.hpp"

#include <string>
#include <unordered_map>

namespace stm::siir {

class MachineOperand;
class MachineInst;
class MachineBasicBlock;
class MachineFunction;

namespace x64 {

/// Recognized x64 opcodes.
enum Opcode : u32 {
    NO_OP = 0x0,

    NOP,
    LEA, 
    CALL, RET, 
    JMP, 
    CQO,

    PUSH64, POP64,
    MOV8, MOV16, MOV32, MOV64,
    ADD8, ADD16, ADD32, ADD64,
    SUB8, SUB16, SUB32, SUB64,
    MUL8, MUL16, MUL32, MUL64,
    IMUL8, IMUL16, IMUL32, IMUL64,
    DIV8, DIV16, DIV32, DIV64,
    IDIV8, IDIV16, IDIV32, IDIV64,
    AND8, AND16, AND32, AND64,
    OR8, OR16, OR32, OR64,
    XOR8, XOR16, XOR32, XOR64,
    SHL8, SHL16, SHL32, SHL64,
    SHR8, SHR16, SHR32, SHR64,
    SAR8, SAR16, SAR32, SAR64,
    CMP8, CMP16, CMP32, CMP64,
    NOT8, NOT16, NOT32, NOT64,
    NEG8, NEG16, NEG32, NEG64,

    JE, JNE, JZ, JNZ,
    JL, JLE, JG, JGE,
    JA, JAE, JB, JBE,

    SETE, SETNE, SETZ, SETNZ,
    SETL, SETLE, SETG, SETGE,
    SETA, SETAE, SETB, SETBE,

    MOVSS, MOVSD,
    MOVAPS, MOVAPD,
    UCOMISS, UCOMISD,
    ADDSS, ADDSD,
    SUBSS, SUBSD,
    MULSS, MULSD,
    DIVSS, DIVSD,
    XORPS, XORPD,

    CVTSS2SD, CVTSD2SS,
    CVTSI2SS, CVTSI2SD,
    CVTTSS2SI8, CVTTSS2SI16, CVTTSS2SI32, CVTTSS2SI64,
    CVTTSD2SI8, CVTTSD2SI16, CVTTSD2SI32, CVTTSD2SI64,
};

/// Recognized x64 physical registers.
enum Register : u32 {
    NO_REG = 0x0,

    RAX, RBX, RCX, RDX,
    RDI, RSI,
    R8, R9, R10, R11, 
    R12, R13, R14, R15,
    RSP, RBP,
    RIP,
    XMM0, XMM1, XMM2, XMM3,
    XMM4, XMM5, XMM6, XMM7,
    XMM8, XMM9, XMM10, XMM11,
    XMM12, XMM13, XMM14, XMM15,
};

/// x64 Instruction selection pass over a SIIR function.
class X64InstSelection final {
    MachineFunction* m_function;
    MachineBasicBlock* m_insert = nullptr;

    /// Temporary mapping between bytecode virtual registers and machine 
    /// register ids.
    std::unordered_map<u32, MachineRegister> m_vregs = {};

    void select(const Instruction* inst);

public:
    X64InstSelection(MachineFunction* function) : m_function(function) {}

    X64InstSelection(const X64InstSelection&) = delete;
    X64InstSelection& operator = (const X64InstSelection&) = delete;

    void run();
};

/// Machine code pass to pretty-print x64 machine objects.
///
/// This pass is not the same as AsmWriter, which emits actually assembly
/// language fit for assembler calls. This pass should instead be used to
/// dump machine IR, exposing certain details.
class X64Printer final {
    const MachineObject& m_obj;

public:
    X64Printer(MachineObject& obj) : m_obj(obj) {}

    X64Printer(const X64Printer&) = delete;
    X64Printer& operator = (const X64Printer&) = delete;

    void run(std::ostream& os) const;
};

/// Machine code pass to emit raw assembly for x64 machine objects.
class X64AsmWriter final {
    const MachineObject& m_obj;

public:
    X64AsmWriter(MachineObject& obj) : m_obj(obj) {}

    X64AsmWriter(const X64AsmWriter&) = delete;
    X64AsmWriter& operator = (const X64AsmWriter&) = delete;

    void run(std::ostream& os) const;
};

/// Returns true if the opcode |op| is considered a call instruction.
bool is_call_opcode(Opcode op);

/// Returns true if the opcode |op| is considered a return instruction.
bool is_ret_opcode(Opcode op);

/// Returns the register class of the physical register |reg|.
RegisterClass get_class(Register reg);

/// Returns true if the physical register |reg| is considered callee-saved.
bool is_callee_saved(Register reg);

/// Returns true if the physical register |reg| is considered caller-saved.
bool is_caller_saved(Register reg);

/// Returns the string representation of the opcode |op|. This is used for
/// dumping purposes, and does not represent the recognized x64 assembly
/// equivelant
std::string to_string(Opcode op);

/// Returns the string representation of the physical register |reg|, with
/// optional x64 subregister |subreg|. This is used for dumping purposes,
/// and does not represent the recognized x64 assembly equivelant.
std::string to_string(Register reg, u16 subreg = 0);

} // namespace x64

} // namespace stm::siir

#endif // STATIM_SIIR_X64_H_
