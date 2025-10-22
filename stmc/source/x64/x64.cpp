#include "x64/x64.hpp"
#include "siir/machine_register.hpp"

#include <cassert>

using namespace stm;
using namespace stm::siir;
using namespace stm::siir::x64;

bool x64::is_call_opcode(x64::Opcode opc) {
    return opc == x64::CALL64;
}

bool x64::is_ret_opcode(x64::Opcode opc) {
    return opc == x64::RET64;
}

bool x64::is_move_opcode(x64::Opcode opc) {
    switch (opc) {
    case MOV:
    case MOV8:
    case MOV16:
    case MOV32:
    case MOV64:
    case MOVSS:
    case MOVSD:
    case MOVAPS:
    case MOVAPD:
        return true;
    default:
        return false;
    }
}

bool x64::is_terminating_opcode(x64::Opcode opc) {
    switch (opc) {
    case JMP:
    case RET64:
    case JE:
    case JNE:
    case JZ:
    case JNZ:
    case JL:
    case JLE:
    case JG:
    case JGE:
    case JA:
    case JAE:
    case JB:
    case JBE:
        return true;
    default:
        return false;
    }
}

RegisterClass x64::get_class(Register reg) {
    switch (reg) {
    case RAX:
    case RBX:
    case RCX:
    case RDX:
    case RDI:
    case RSI:
    case R8:
    case R9:
    case R10:
    case R11:
    case R12:
    case R13:
    case R14:
    case R15:
    case RSP:
    case RBP:
    case RIP:
        return GeneralPurpose;
    case XMM0:
    case XMM1:
    case XMM2:
    case XMM3:
    case XMM4:
    case XMM5:
    case XMM6:
    case XMM7:
    case XMM8:
    case XMM9:
    case XMM10:
    case XMM11:
    case XMM12:
    case XMM13:
    case XMM14:
    case XMM15:
        return FloatingPoint;
    default:
        assert(false && "unrecognied x64 physical register!");
    }
}

bool x64::is_callee_saved(Register reg) {
    switch (reg) {
    case RBX:
    case R12:
    case R13:
    case R14:
    case R15:
    case RSP:
    case RBP:
        return true;
    default:
        return false;
    }
}

bool x64::is_caller_saved(Register reg) {
    switch (reg) {
    case RAX:
    case RCX:
    case RDX:
    case RDI:
    case RSI:
    case R8:
    case R9:
    case R10:
    case R11:
    case R12:
    case R13:
    case R14:
    case R15:
    case XMM0:
    case XMM1:
    case XMM2:
    case XMM3:
    case XMM4:
    case XMM5:
    case XMM6:
    case XMM7:
    case XMM8:
    case XMM9:
    case XMM10:
    case XMM11:
    case XMM12:
    case XMM13:
    case XMM14:
    case XMM15:
        return true;
    default:
        return false;
    }
}

TargetRegisters x64::get_registers() {
    RegisterSet gpr;
    gpr.cls = GeneralPurpose;
    gpr.regs = {
        RAX, RCX, RDX, RSI, RDI, R8, R9, 
        R10, R11, R12, R13, R14, R15
    };

    RegisterSet fpr;
    fpr.cls = FloatingPoint;
    fpr.regs = {
        XMM0, XMM1, XMM2, XMM3,
        XMM4, XMM5, XMM6, XMM7,
        XMM8, XMM9, XMM10, XMM11,
        XMM12, XMM13, XMM14, XMM15
    };

    TargetRegisters tregs;
    tregs.regs[GeneralPurpose] = gpr;
    tregs.regs[FloatingPoint] = fpr;
    return tregs;
}

std::string x64::to_string(Opcode op) {
    switch (op) {
    case NOP:
        return "NOP";
    case JMP:
        return "JMP";
    case UD2:
        return "UD2";
    case CQO:
        return "CQO";
    case MOV:
        return "MOV";
    case CALL64:
        return "CALL64";
    case RET64:
        return "RET64";
    case LEA32:
        return "LEA32";
    case LEA64:
        return "LEA64";
    case PUSH64:
        return "PUSH64";
    case POP64:
        return "POP64";
    case MOV8:
        return "MOV8";
    case MOV16:
        return "MOV16";
    case MOV32:
        return "MOV32";
    case MOV64:
        return "MOV64";
    case ADD8:
        return "ADD8";
    case ADD16:
        return "ADD16";
    case ADD32:
        return "ADD32";
    case ADD64:
        return "ADD64";
    case SUB8:
        return "SUB8";
    case SUB16:
        return "SUB16";
    case SUB32:
        return "SUB32";
    case SUB64:
        return "SUB64";
    case MUL8:
        return "MUL8";
    case MUL16:
        return "MUL16";
    case MUL32:
        return "MUL32";
    case MUL64:
        return "MUL64";
    case IMUL8:
        return "IMUL8";
    case IMUL16:
        return "IMUL16";
    case IMUL32:
        return "IMUL32";
    case IMUL64:
        return "IMUL64";
    case DIV8:
        return "DIV8";
    case DIV16:
        return "DIV16";
    case DIV32:
        return "DIV32";
    case DIV64:
        return "DIV64";
    case IDIV8:
        return "IDIV8";
    case IDIV16:
        return "IDIV16";
    case IDIV32:
        return "IDIV32";
    case IDIV64:
        return "IDIV64";
    case AND8:
        return "AND8";
    case AND16:
        return "AND16";
    case AND32:
        return "AND32";
    case AND64:
        return "AND64";
    case OR8:
        return "OR8";
    case OR16:
        return "OR16";
    case OR32:
        return "OR32";
    case OR64:
        return "OR64";
    case XOR8:
        return "XOR8";
    case XOR16:
        return "XOR16";
    case XOR32:
        return "XOR32";
    case XOR64:
        return "XOR64";
    case SHL8:
        return "SHL8";
    case SHL16:
        return "SHL16";
    case SHL32:
        return "SHL32";
    case SHL64:
        return "SHL64";
    case SHR8:
        return "SHR8";
    case SHR16:
        return "SHR16";
    case SHR32:
        return "SHR32";
    case SHR64:
        return "SHR64";
    case SAR8:
        return "SAR8";
    case SAR16:
        return "SAR16";
    case SAR32:
        return "SAR32";
    case SAR64:
        return "SAR64";
    case CMP8:
        return "CMP8";
    case CMP16:
        return "CMP16";
    case CMP32:
        return "CMP32";
    case CMP64:
        return "CMP64";
    case NOT8:
        return "NOT8";
    case NOT16:
        return "NOT16";
    case NOT32:
        return "NOT32";
    case NOT64:
        return "NOT64";
    case NEG8:
        return "NEG8";
    case NEG16:
        return "NEG16";
    case NEG32:
        return "NEG32";
    case NEG64:
        return "NEG64";
    case MOVABS:
        return "MOVABS";
    case MOVSX:
        return "MOVSX";
    case MOVSXD:
        return "MOVSXD";
    case MOVZX:
        return "MOVZX";
    case JE:
        return "JE";
    case JNE:
        return "JNE";
    case JZ:
        return "JZ";
    case JNZ:
        return "JNZ";
    case JL:
        return "JL";
    case JLE:
        return "JLE";
    case JG:
        return "JG";
    case JGE:
        return "JGE";
    case JA:
        return "JA";
    case JAE:
        return "JAE";
    case JB:
        return "JB";
    case JBE:
        return "JBE";
    case SETE:
        return "SETE";
    case SETNE:
        return "SETNE";
    case SETZ:
        return "SETZ";
    case SETNZ:
        return "SETNZ";
    case SETL:
        return "SETL";
    case SETLE:
        return "SETLE";
    case SETG:
        return "SETG";
    case SETGE:
        return "SETGE";
    case SETA:
        return "SETA";
    case SETAE:
        return "SETAE";
    case SETB:
        return "SETB";
    case SETBE:
        return "SETBE";
    case MOVSS:
        return "MOVSS";
    case MOVSD:
        return "MOVSD";
    case MOVAPS:
        return "MOVAPS";
    case MOVAPD:
        return "MOVAPD";
    case UCOMISS:
        return "UCOMISS";
    case UCOMISD:
        return "UCOMISD";
    case ADDSS:
        return "ADDSS";
    case ADDSD:
        return "ADDSD";
    case SUBSS:
        return "SUBSS";
    case SUBSD:
        return "SUBSD";
    case MULSS:
        return "MULSS";
    case MULSD:
        return "MULSD";
    case DIVSS:
        return "DIVSS";
    case DIVSD:
        return "DIVSD";
    case XORPS:
        return "XORPS";
    case XORPD:
        return "XORPD";
    case CVTSS2SD:
        return "CVTSS2SD";
    case CVTSD2SS:
        return "CVTSD2SS";
    case CVTSI2SS:
        return "CVTSI2SS";
    case CVTSI2SD:
        return "CVTSI2SD";
    case CVTTSS2SI8:
        return "CVTTSS2SI8";
    case CVTTSS2SI16:
        return "CVTTSS2SI16";
    case CVTTSS2SI32:
        return "CVTTSS2SI32";
    case CVTTSS2SI64:
        return "CVTTSS2SI64";
    case CVTTSD2SI8:
        return "CVTTSD2SI8";
    case CVTTSD2SI16:
        return "CVTTSD2SI16";
    case CVTTSD2SI32:
        return "CVTTSD2SI32";
    case CVTTSD2SI64:
        return "CVTTSD2SI64";
    default:
        assert(false && "unrecognized x64 opcode!");
    }
}

std::string x64::to_string(Register reg, u16 subreg) {
    switch (reg) {
    case RAX:  
        return subreg == 8 ? "rax" : (subreg == 4 ? "eax" : 
            (subreg == 2 ? "ax"  : (subreg == 1 ? "al"  : "ah")));
    case RBX:  
        return subreg == 8 ? "rbx" : (subreg == 4 ? "ebx" : 
            (subreg == 2 ? "bx"  : (subreg == 1 ? "bl"  : "bh")));
    case RCX:  
        return subreg == 8 ? "rcx" : (subreg == 4 ? "ecx" : 
            (subreg == 2 ? "cx"  : (subreg == 1 ? "cl"  : "ch")));
    case RDX:  
        return subreg == 8 ? "rdx" : (subreg == 4 ? "edx" : 
            (subreg == 2 ? "dx"  : (subreg == 1 ? "dl"  : "dh")));
    case RDI:  
        return subreg == 8 ? "rdi" : (subreg == 4 ? "edi" : 
            (subreg == 2 ? "di"  : (subreg == 1 ? "dil" : "")));
    case RSI:  
        return subreg == 8 ? "rsi" : (subreg == 4 ? "esi" : 
            (subreg == 2 ? "si"  : (subreg == 1 ? "sil" : "")));
    case RBP:  
        return subreg == 8 ? "rbp" : (subreg == 4 ? "ebp" : 
            (subreg == 2 ? "bp"  : (subreg == 1 ? "bpl" : "")));
    case RSP:  
        return subreg == 8 ? "rsp" : (subreg == 4 ? "esp" : 
            (subreg == 2 ? "sp"  : (subreg == 1 ? "spl" : "")));
    case R8:   
        return subreg == 8 ? "r8" : (subreg == 4 ? "r8d" : 
            (subreg == 2 ? "r8w" : (subreg == 1 ? "r8b" : "")));
    case R9:   
        return subreg == 8 ? "r9" : (subreg == 4 ? "r9d" :
             (subreg == 2 ? "r9w" : (subreg == 1 ? "r9b" : "")));
    case R10:  
        return subreg == 8 ? "r10" : (subreg == 4 ? "r10d" : 
            (subreg == 2 ? "r10w": (subreg == 1 ? "r10b": "")));
    case R11:  
        return subreg == 8 ? "r11" : (subreg == 4 ? "r11d" : 
            (subreg == 2 ? "r11w": (subreg == 31 ? "r11b": "")));
    case R12:  
        return subreg == 8 ? "r12" : (subreg == 4 ? "r12d" : 
            (subreg == 2 ? "r12w": (subreg == 1 ? "r12b": "")));
    case R13:  
        return subreg == 8 ? "r13" : (subreg == 4 ? "r13d" : 
            (subreg == 2 ? "r13w": (subreg == 1 ? "r13b": "")));
    case R14:  
        return subreg == 8 ? "r14" : (subreg == 4 ? "r14d" : 
            (subreg == 2 ? "r14w": (subreg == 1 ? "r14b": "")));
    case R15:  
        return subreg == 8 ? "r15" : (subreg == 4 ? "r15d" : 
            (subreg == 2 ? "r15w": (subreg == 1 ? "r15b": "")));
    case RIP:  
        return "rip";
    case XMM0: 
        return "xmm0";
    case XMM1: 
        return "xmm1";
    case XMM2: 
        return "xmm2";
    case XMM3: 
        return "xmm3";
    case XMM4: 
        return "xmm4";
    case XMM5: 
        return "xmm5";
    case XMM6: 
        return "xmm6";
    case XMM7: 
        return "xmm7";
    case XMM8: 
        return "xmm8";
    case XMM9: 
        return "xmm9";
    case XMM10:
        return "xmm10";
    case XMM11:
        return "xmm11";
    case XMM12:
        return "xmm12";
    case XMM13:
        return "xmm13";
    case XMM14:
        return "xmm14";
    case XMM15:
        return "xmm15";
    default:   
        assert(false && "unrecognized x64 physical register!");
    }
}
