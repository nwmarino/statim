//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/AMD64.hpp"

#include <cassert>

using namespace lir;

const char *lir::to_string(AMD64_Op op) {
    switch (op) {
        case AMD64_NOP:
            return "AMD64NOP";
        case AMD64_JMP:
            return "AMD64JMP";
        case AMD64_UD2:
        case AMD64_CQO:
        case AMD64_SYSCALL:
        case AMD64_MOV:
        case AMD64_MOVZX:
        case AMD64_MOVSX:
        case AMD64_MOVSXD:
        case AMD64_CALL32:
        case AMD64_CALL64:
        case AMD64_RET32:
        case AMD64_RET64:
        case AMD64_LEA32:
        case AMD64_LEA64:
        case AMD64_PUSH32:
        case AMD64_PUSH64:
        case AMD64_POP32:
        case AMD64_POP64:
        case AMD64_MOV8:
        case AMD64_MOV16:
        case AMD64_MOV32:
        case AMD64_MOV64:
        case AMD64_ADD8:
        case AMD64_ADD16:
        case AMD64_ADD32:
        case AMD64_ADD64:
        case AMD64_SUB8:
        case AMD64_SUB16:
        case AMD64_SUB32:
        case AMD64_SUB64:
        case AMD64_MUL8:
        case AMD64_MUL16:
        case AMD64_MUL32:
        case AMD64_MUL64:
        case AMD64_IMUL8:
        case AMD64_IMUL16:
        case AMD64_IMUL32:
        case AMD64_IMUL64:
        case AMD64_DIV8:
        case AMD64_DIV16:
        case AMD64_DIV32:
        case AMD64_DIV64:
        case AMD64_IDIV8:
        case AMD64_IDIV16:
        case AMD64_IDIV32:
        case AMD64_IDIV64:
        case AMD64_AND8:
        case AMD64_AND16:
        case AMD64_AND32:
        case AMD64_AND64:
        case AMD64_OR8:
        case AMD64_OR16:
        case AMD64_OR32:
        case AMD64_OR64:
        case AMD64_XOR8:
        case AMD64_XOR16:
        case AMD64_XOR32:
        case AMD64_XOR64:
        case AMD64_SHL8:
        case AMD64_SHL16:
        case AMD64_SHL32:
        case AMD64_SHL64:
        case AMD64_SHR8:
        case AMD64_SHR16:
        case AMD64_SHR32:
        case AMD64_SHR64:
        case AMD64_SAR8:
        case AMD64_SAR16:
        case AMD64_SAR32:
        case AMD64_SAR64:
        case AMD64_CMP8:
        case AMD64_CMP16:
        case AMD64_CMP32:
        case AMD64_CMP64:
        case AMD64_NOT8:
        case AMD64_NOT16:
        case AMD64_NOT32:
        case AMD64_NOT64:
        case AMD64_NEG8:
        case AMD64_NEG16:
        case AMD64_NEG32:
        case AMD64_NEG64:
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
        case AMD64_SETE:
        case AMD64_SETNE:
        case AMD64_SETZ:
        case AMD64_SETNZ:
        case AMD64_SETL:
        case AMD64_SETLE:
        case AMD64_SETG:
        case AMD64_SETGE:
        case AMD64_SETA:
        case AMD64_SETAE:
        case AMD64_SETB:
        case AMD64_SETBE:
        case AMD64_MOVSS:
        case AMD64_MOVSD:
        case AMD64_MOVAPS:
        case AMD64_MOVAPD:
        case AMD64_UCOMISS:
        case AMD64_UCOMISD:
        case AMD64_ADDSS:
        case AMD64_ADDSD:
        case AMD64_SUBSS:
        case AMD64_SUBSD:
        case AMD64_MULSS:
        case AMD64_MULSD:
        case AMD64_DIVSS:
        case AMD64_DIVSD:
        case AMD64_ANDPS:
        case AMD64_ANDPD:
        case AMD64_ORPS:
        case AMD64_ORPD:
        case AMD64_XORPS:
        case AMD64_XORPD:
        case AMD64_CVTSS2SD:
        case AMD64_CVTSD2SS:
        case AMD64_CVTSI2SS:
        case AMD64_CVTSI2SD:
        case AMD64_CVTTSS2SI8:
        case AMD64_CVTTSS2SI16:
        case AMD64_CVTTSS2SI32:
        case AMD64_CVTTSS2SI64:
        case AMD64_CVTTSD2SI8:
        case AMD64_CVTTSD2SI16:
        case AMD64_CVTTSD2SI32:
        case AMD64_CVTTSD2SI64:
        case AMD64_VCVCTSS2USI8:
        case AMD64_VCVCTSS2USI16:
        case AMD64_VCVCTSS2USI32:
        case AMD64_VCVCTSS2USI64:
            return "AMD64VCVTSS2USI64";
    }
}

const char *lir::to_string(AMD64_Register reg, uint16_t subreg) {
    switch (reg) {
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
