#ifndef STATIM_MACHINE_AMD64_HPP_
#define STATIM_MACHINE_AMD64_HPP_

#include "siir/instruction.hpp"
#include "types/types.hpp"
#include "machine/register.hpp"
#include "machine/object.hpp"

#include <string>
#include <unordered_map>

namespace stm {

class MachineOperand;
class MachineInst;
class MachineBasicBlock;
class MachineFunction;

namespace amd64 {

/// Recognized amd64 opcodes.
enum Opcode : u32 {
    NOP = 0x0,
    
    LEA, CALL, RET, JMP, CQO,

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

/// Recognized amd64 physical registers.
enum Register : u32 {
    NONE = 0x0,
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

/// Instruction selection pass for amd64.
class InstSelection final {
    MachineFunction* m_function;
    MachineBasicBlock* m_insert = nullptr;

    /// Temporary mapping between bytecode virtual registers and machine 
    /// register ids.
    std::unordered_map<u32, MachineRegister> m_vregs = {};

    /// \returns The operand flipped jcc instruction of \p jcc.
    amd64::Opcode flip_jcc(amd64::Opcode jcc);

    /// \returns The negated jcc instruction of \p jcc.
    amd64::Opcode neg_jcc(amd64::Opcode jcc);

    /// \returns The operand flipped setcc instruction of \p setcc.
    amd64::Opcode flip_setcc(amd64::Opcode setcc);

    /// \returns The negated setcc instruction of \p setcc.
    amd64::Opcode neg_setcc(amd64::Opcode setcc);

    void select(siir::Instruction* inst);

public:
    InstSelection(MachineFunction* function);

    InstSelection(const InstSelection&) = delete;
    InstSelection& operator = (const InstSelection&) = delete;

    void run();
};

/// Machine code pass to dump amd64, exposing important details.
class Printer final {
    const MachineObject& m_obj;

public:
    Printer(MachineObject& obj) : m_obj(obj) {}

    Printer(const Printer&) = delete;
    Printer& operator = (const Printer&) = delete;

    void run(std::ostream& os) const;
};

/// Machine code pass to write final assembly output for amd64.
class AsmWriter final {
    const MachineObject& m_obj;

public:
    AsmWriter(MachineObject& obj) : m_obj(obj) {}

    AsmWriter(const AsmWriter&) = delete;
    AsmWriter& operator = (const AsmWriter&) = delete;

    void run(std::ostream& os) const;
};

/// \returns `true` if the opcode is considered a call instruction.
bool is_call_opcode(amd64::Opcode opcode);

/// \returns `true` if the opcode is considered a return instruction.
bool is_ret_opcode(amd64::Opcode opcode);

/// \returns The register class of physical register \p reg.
RegisterClass get_class(amd64::Register reg);

/// \returns `true` if the physical register \p reg is considered callee-saved.
bool is_callee_saved(amd64::Register reg);

/// \returns `true` if the physical register \p reg is considered caller-saved.
bool is_caller_saved(amd64::Register reg);

/// \returns A string representation of \p opcode.
std::string to_string(amd64::Opcode opcode);

/// \returns A string representation of \p reg.
std::string to_string(amd64::Register reg, u16 subreg = 0);

} // namespace amd64

} // namespace stm

#endif // STATIM_MACHINE_AMD64_HPP_
