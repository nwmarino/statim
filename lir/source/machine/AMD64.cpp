//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/AMD64.h"

#include <cassert>

using namespace lir;

const char* lir::to_string(AMD64_Op op) {
    switch (op)
    {
        case AMD64_NOP:
            return "AMD64NOP";
        case AMD64_JMP:
            return "AMD64JMP";
        case AMD64_UD2:
            return "AMD64UD2";
        case AMD64_CQO:
            return "AMD64CQO";
        case AMD64_SYSCALL:
            return "AMD64SYSCALL";
        case AMD64_MOV:
            return "AMD64MOV";
        case AMD64_MOVZX:
            return "AMD64MOVZX";
        case AMD64_MOVSX:
            return "AMD64MOVSX";
        case AMD64_MOVSXD:
            return "AMD64MOVSXD";
        case AMD64_MOVABS:
            return "AMD64MOVABS";
        case AMD64_CALL32:
            return "AMD64CALL32";
        case AMD64_CALL64:
            return "AMD64CALL64";
        case AMD64_RET32:
            return "AMD64RET32";
        case AMD64_RET64:
            return "AMD64RET64";
        case AMD64_LEA32:
            return "AMD64LEA32";
        case AMD64_LEA64:
            return "AMD64LEA64";
        case AMD64_PUSH32:
            return "AMD64PUSH32";
        case AMD64_PUSH64:
            return "AMD64PUSH64";
        case AMD64_POP32:
            return "AMD64POP32";
        case AMD64_POP64:
            return "AMD64POP64";
        case AMD64_MOV8:
            return "AMD64MOV8";
        case AMD64_MOV16:
            return "AMD64MOV16";
        case AMD64_MOV32:
            return "AMD64MOV32";
        case AMD64_MOV64:
            return "AMD64MOV64";
        case AMD64_ADD8:
            return "AMD64ADD8";
        case AMD64_ADD16:
            return "AMD64ADD16";
        case AMD64_ADD32:
            return "AMD64ADD32";
        case AMD64_ADD64:
            return "AMD64ADD64";
        case AMD64_SUB8:
            return "AMD64SUB8";
        case AMD64_SUB16:
            return "AMD64SUB16";
        case AMD64_SUB32:
            return "AMD64SUB32";
        case AMD64_SUB64:
            return "AMD64SUB64";
        case AMD64_MUL8:
            return "AMD64MUL8";
        case AMD64_MUL16:
            return "AMD64MUL16";
        case AMD64_MUL32:
            return "AMD64MUL32";
        case AMD64_MUL64:
            return "AMD64MUL64";
        case AMD64_IMUL8:
            return "AMD64IMUL8";
        case AMD64_IMUL16:
            return "AMD64IMUL16";
        case AMD64_IMUL32:
            return "AMD64IMUL32";
        case AMD64_IMUL64:
            return "AMD64IMUL64";
        case AMD64_DIV8:
            return "AMD64DIV8";
        case AMD64_DIV16:
            return "AMD64DIV16";
        case AMD64_DIV32:
            return "AMD64DIV32";
        case AMD64_DIV64:
            return "AMD64DIV64";
        case AMD64_IDIV8:
            return "AMD64IDIV8";
        case AMD64_IDIV16:
            return "AMD64IDIV16";
        case AMD64_IDIV32:
            return "AMD64IDIV32";
        case AMD64_IDIV64:
            return "AMD64IDIV64";
        case AMD64_AND8:
            return "AMD64AND8";
        case AMD64_AND16:
            return "AMD64AND16";
        case AMD64_AND32:
            return "AMD64AND32";
        case AMD64_AND64:
            return "AMD64AND64";
        case AMD64_OR8:
            return "AMD64OR8";
        case AMD64_OR16:
            return "AMD64OR16";
        case AMD64_OR32:
            return "AMD64OR32";
        case AMD64_OR64:
            return "AMD64OR64";
        case AMD64_XOR8:
            return "AMD64XOR8";
        case AMD64_XOR16:
            return "AMD64XOR16";
        case AMD64_XOR32:
            return "AMD64XOR32";
        case AMD64_XOR64:
            return "AMD64XOR64";
        case AMD64_SHL8:
            return "AMD64SHL8";
        case AMD64_SHL16:
            return "AMD64SHL16";
        case AMD64_SHL32:
            return "AMD64SHL32";
        case AMD64_SHL64:
            return "AMD64SHL64";
        case AMD64_SHR8:
            return "AMD64SHR8";
        case AMD64_SHR16:
            return "AMD64SHR16";
        case AMD64_SHR32:
            return "AMD64SHR32";
        case AMD64_SHR64:
            return "AMD64SHR64";
        case AMD64_SAR8:
            return "AMD64SAR8";
        case AMD64_SAR16:
            return "AMD64SAR16";
        case AMD64_SAR32:
            return "AMD64SAR32";
        case AMD64_SAR64:
            return "AMD64SAR64";
        case AMD64_CMP8:
            return "AMD64CMP8";
        case AMD64_CMP16:
            return "AMD64CMP16";
        case AMD64_CMP32:
            return "AMD64CMP32";
        case AMD64_CMP64:
            return "AMD64CMP64";
        case AMD64_NOT8:
            return "AMD64NOT8";
        case AMD64_NOT16:
            return "AMD64NOT16";
        case AMD64_NOT32:
            return "AMD64NOT32";
        case AMD64_NOT64:
            return "AMD64NOT64";
        case AMD64_NEG8:
            return "AMD64NEG8";
        case AMD64_NEG16:
            return "AMD64NEG16";
        case AMD64_NEG32:
            return "AMD64NEG32";
        case AMD64_NEG64:
            return "AMD64NEG64";
        case AMD64_JE:
            return "AMD64JE";
        case AMD64_JNE:
            return "AMD64JNE";
        case AMD64_JZ:
            return "AMD64JZ";
        case AMD64_JNZ:
            return "AMD64JNZ";
        case AMD64_JL:
            return "AMD64JL";
        case AMD64_JLE:
            return "AMD64JLE";
        case AMD64_JG:
            return "AMD64JG";
        case AMD64_JGE:
            return "AMD64JGE";
        case AMD64_JA:
            return "AMD64JA";
        case AMD64_JAE:
            return "AMD64JAE";
        case AMD64_JB:
            return "AMD64JB";
        case AMD64_JBE:
            return "AMD64JBE";
        case AMD64_SETE:
            return "AMD64SETE";
        case AMD64_SETNE:
            return "AMD64SETNE";
        case AMD64_SETZ:
            return "AMD64SETZ";
        case AMD64_SETNZ:
            return "AMD64SETNZ";
        case AMD64_SETL:
            return "AMD64SETL";
        case AMD64_SETLE:
            return "AMD64SETLE";
        case AMD64_SETG:
            return "AMD64SETG";
        case AMD64_SETGE:
            return "AMD64SETGE";
        case AMD64_SETA:
            return "AMD64SETA";
        case AMD64_SETAE:
            return "AMD64SETAE";
        case AMD64_SETB:
            return "AMD64SETB";
        case AMD64_SETBE:
            return "AMD64SETBE";
        case AMD64_MOVSS:
            return "AMD64MOVSS";
        case AMD64_MOVSD:
            return "AMD64MOVSD";
        case AMD64_MOVAPS:
            return "AMD64MOVAPS";
        case AMD64_MOVAPD:
            return "AMD64MOVAPD";
        case AMD64_UCOMISS:
            return "AMD64UCOMISS";
        case AMD64_UCOMISD:
            return "AMD64UCOMISD";
        case AMD64_ADDSS:
            return "AMD64ADDSS";
        case AMD64_ADDSD:
            return "AMD64ADDSD";
        case AMD64_SUBSS:
            return "AMD64SUBSS";
        case AMD64_SUBSD:
            return "AMD64SUBSD";
        case AMD64_MULSS:
            return "AMD64MULSS";
        case AMD64_MULSD:
            return "AMD64MULSD";
        case AMD64_DIVSS:
            return "AMD64DIVSS";
        case AMD64_DIVSD:
            return "AMD64DIVSD";
        case AMD64_ANDPS:
            return "AMD64ANDPS";
        case AMD64_ANDPD:
            return "AMD64ANDPD";
        case AMD64_ORPS:
            return "AMD64ORPS";
        case AMD64_ORPD:
            return "AMD64ORPD";
        case AMD64_XORPS:
            return "AMD64XORPS";
        case AMD64_XORPD:
            return "AMD64XORPD";
        case AMD64_CVTSS2SD:
            return "AMD64CVTSS2SD";
        case AMD64_CVTSD2SS:
            return "AMD64CVTSD2SS";
        case AMD64_CVTSI2SS:
            return "AMD64CVTSI2SS";
        case AMD64_CVTSI2SD:
            return "AMD64CVTSI2SD";
        case AMD64_VCVTUSI2SS:
            return "AMD64VCVTUSI2SS";
        case AMD64_VCVTUSI2SD:
            return "AMD64VCVTUSI2SD";
        case AMD64_CVTTSS2SI8:
            return "AMD64CVTTSS2SI8";
        case AMD64_CVTTSS2SI16:
            return "AMD64CVTTSS2SI16";
        case AMD64_CVTTSS2SI32:
            return "AMD64CVTTSS2SI32";
        case AMD64_CVTTSS2SI64:
            return "AMD64CVTTSS2SI64";
        case AMD64_CVTTSD2SI8:
            return "AMD64CVTTSD2SI8";
        case AMD64_CVTTSD2SI16:
            return "AMD64CVTTSD2SI16";
        case AMD64_CVTTSD2SI32:
            return "AMD64CVTTSD2SI32";
        case AMD64_CVTTSD2SI64:
            return "AMD64CVTTSD2SI64";
        case AMD64_VCVCTSS2USI8:
            return "AMD64VCVTSS2USI8";
        case AMD64_VCVCTSS2USI16:
            return "AMD64VCVTSS2USI16";
        case AMD64_VCVCTSS2USI32:
            return "AMD64VCVTSS2USI32";
        case AMD64_VCVCTSS2USI64:
            return "AMD64VCVTSS2USI64";
        case AMD64_VCVCTSD2USI8:
            return "AMD64VCVCTSD2USI8";
        case AMD64_VCVCTSD2USI16:
            return "AMD64VCVCTSD2USI16";
        case AMD64_VCVCTSD2USI32:
            return "AMD64VCVCTSD2USI32";
        case AMD64_VCVCTSD2USI64:
            return "AMD64VCVCTSD2USI64";
    }
}

const char* lir::to_string(AMD64_Register reg, uint8_t subreg) {
    switch (reg) 
    {
        case RAX:
            switch (subreg) {
                case 8: return "rax";
                case 4: return "eax";
                case 2: return "ax";
                case 1: return "al";
                default: return "rax";
            }
        case RBX:
            switch (subreg) {
                case 8: return "rbx";
                case 4: return "ebx";
                case 2: return "bx";
                case 1: return "bl";
                default: return "rbx";
            }
        case RCX:
            switch (subreg) {
                case 8: return "rcx";
                case 4: return "ecx";
                case 2: return "cx";
                case 1: return "cl";
                default: return "rcx";
            }
        case RDX:
            switch (subreg) {
                case 8: return "rdx";
                case 4: return "edx";
                case 2: return "dx";
                case 1: return "dl";
                default: return "rdx";
            }
        case RDI:
            switch (subreg) {
                case 8: return "rdi";
                case 4: return "edi";
                case 2: return "di";
                case 1: return "dil";
                default: return "rdi";
            }
        case RSI:
            switch (subreg) {
                case 8: return "rsi";
                case 4: return "esi";
                case 2: return "si";
                case 1: return "sil";
                default: return "rsi";
            }
        case RBP:
            switch (subreg) {
                case 8: return "rbp";
                case 4: return "ebp";
                case 2: return "bp";
                case 1: return "bpl";
                default: return "rbp";
            }
        case RSP:
            switch (subreg) {
                case 8: return "rsp";
                case 4: return "esp";
                case 2: return "sp";
                case 1: return "spl";
                default: return "rsp";
            }
        case R8:
            switch (subreg) {
                case 8: return "r8";
                case 4: return "r8d";
                case 2: return "r8w";
                case 1: return "r8b";
                default: return "r8";
            }
        case R9:
            switch (subreg) {
                case 8: return "r9";
                case 4: return "r9d";
                case 2: return "r9w";
                case 1: return "r9b";
                default: return "r9";
            }
        case R10:
            switch (subreg) {
                case 8: return "r10";
                case 4: return "r10d";
                case 2: return "r10w";
                case 1: return "r10b";
                default: return "r10";
            }
        case R11:
            switch (subreg) {
                case 8: return "r11";
                case 4: return "r11d";
                case 2: return "r11w";
                case 1: return "r11b";
                default: return "r11";
            }
        case R12:
            switch (subreg) {
                case 8: return "r12";
                case 4: return "r12d";
                case 2: return "r12w";
                case 1: return "r12b";
                default: return "";
            }
        case R13:
            switch (subreg) {
                case 8: return "r13";
                case 4: return "r13d";
                case 2: return "r13w";
                case 1: return "r13b";
                default: return "r13";
            }
        case R14:
            switch (subreg) {
                case 8: return "r14";
                case 4: return "r14d";
                case 2: return "r14w";
                case 1: return "r14b";
                default: return "r14";
            }
        case R15:
            switch (subreg) {
                case 8: return "r15";
                case 4: return "r15d";
                case 2: return "r15w";
                case 1: return "r15b";
                default: return "r15";
            }
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
            assert(false && "invalid register!");
    }
}

bool lir::is_terminator(AMD64_Op op) {
    switch (op)
    {
    case AMD64_JMP:
    case AMD64_RET32:
    case AMD64_RET64:
    case AMD64_JE:
    case AMD64_JNE:
    case AMD64_JZ:
    case AMD64_JNZ:
    case AMD64_JL:
    case AMD64_JLE:
    case AMD64_JG:
    case AMD64_JGE:
    case AMD64_JA:
    case AMD64_JAE:
    case AMD64_JB:
    case AMD64_JBE:
        return true;
    default:
        return false;
    }
}
