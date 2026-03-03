//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/AsmWriter.h"
#include "lir/machine/MachineConstant.h"
#include "lir/machine/MachineFunction.h"
#include "lir/machine/MachineObject.h"
#include "lir/machine/MachineOp.h"
#include "lir/machine/MachineOperand.h"
#include "lir/machine/MachineRegister.h"

#include <cstring>
#include <format>

using namespace lir;

AsmWriter::AsmWriter(const MachineObject& obj) : m_obj(obj) {}

void AsmWriter::run(std::ostream& os) {
    for (const auto& [name, data] : m_obj.get_globals()) {
        writeData(os, *data);
    }

    for (const auto& [name, func] : m_obj.get_functions()) {
        if (func->empty())
            continue;

        writeFunction(os, *func);
    }
}

void AsmWriter::writeOpcode(std::ostream& os, AMD64_Op op) {
    switch (op) 
    {
    case AMD64_NOP:
        os << "nop";
        return;
    case AMD64_JMP:
        os << "jmp";
        return;
    case AMD64_UD2:
        os << "ud2";
        return;
    case AMD64_CQO:
        os << "cqo";
        return;
    case AMD64_SYSCALL:
        os << "syscall";
        return;
    case AMD64_MOV:
        os << "mov";
        return;
    case AMD64_MOV8:
        os << "movb";
        return;
    case AMD64_MOV16:
        os << "movw";
        return;
    case AMD64_MOV32:
        os << "movl";
        return;
    case AMD64_MOV64:
        os << "movq";
        return;
    case AMD64_MOVZX:
        os << "movzx";
        return;
    case AMD64_MOVSX:
        os << "movsx";
        return;
    case AMD64_MOVSXD:
        os << "movsxd";
        return;
    case AMD64_MOVABS:
        os << "movabs";
        return;
    case AMD64_CALL32:
        os << "calll";
        return;
    case AMD64_CALL64:
        os << "callq";
        return;
    case AMD64_RET32:
        os << "retl";
        return;
    case AMD64_RET64:
        os << "retq";
        return;
    case AMD64_LEA32:
        os << "leal";
        return;
    case AMD64_LEA64:
        os << "leaq";
        return;
    case AMD64_PUSH32:
        os << "pushl";
        return;
    case AMD64_PUSH64:
        os << "pushq";
        return;
    case AMD64_POP32:
        os << "popl";
        return;
    case AMD64_POP64:
        os << "popq";
        return;
    case AMD64_ADD8:
        os << "addb";
        return;
    case AMD64_ADD16:
        os << "addw";
        return;
    case AMD64_ADD32:
        os << "addl";
        return;
    case AMD64_ADD64:
        os << "addq";
        return;
    case AMD64_SUB8:
        os << "subb";
        return;
    case AMD64_SUB16:
        os << "subw";
        return;
    case AMD64_SUB32:
        os << "subl";
        return;
    case AMD64_SUB64:
        os << "subq";
        return;
    case AMD64_MUL8:
        os << "mulb";
        return;
    case AMD64_MUL16:
        os << "mulw";
        return;
    case AMD64_MUL32:
        os << "mull";
        return;
    case AMD64_MUL64:
        os << "mulq";
        return;
    case AMD64_IMUL8:
        os << "imulb";
        return;
    case AMD64_IMUL16:
        os << "imulw";
        return;
    case AMD64_IMUL32:
        os << "imull";
        return;
    case AMD64_IMUL64:
        os << "imulq";
        return;
    case AMD64_DIV8:
        os << "divb";
        return;
    case AMD64_DIV16:
        os << "divw";
        return;
    case AMD64_DIV32:
        os << "divl";
        return;
    case AMD64_DIV64:
        os << "divq";
        return;
    case AMD64_IDIV8:
        os << "idivb";
        return;
    case AMD64_IDIV16:
        os << "idivw";
        return;
    case AMD64_IDIV32:
        os << "idivl";
        return;
    case AMD64_IDIV64:
        os << "idivq";
        return;
    case AMD64_AND8:
        os << "andb";
        return;
    case AMD64_AND16:
        os << "andw";
        return;
    case AMD64_AND32:
        os << "andl";
        return;
    case AMD64_AND64:
        os << "andq";
        return;
    case AMD64_OR8:
        os << "orb";
        return;
    case AMD64_OR16:
        os << "orw";
        return;
    case AMD64_OR32:
        os << "orl";
        return;
    case AMD64_OR64:
        os << "orq";
        return;
    case AMD64_XOR8:
        os << "xorb";
        return;
    case AMD64_XOR16:
        os << "xorw";
        return;
    case AMD64_XOR32:
        os << "xorl";
        return;
    case AMD64_XOR64:
        os << "xorq";
        return;
    case AMD64_SHL8:
        os << "shlb";
        return;
    case AMD64_SHL16:
        os << "shlw";
        return;
    case AMD64_SHL32:
        os << "shll";
        return;
    case AMD64_SHL64:
        os << "shlq";
        return;
    case AMD64_SHR8:
        os << "shrb";
        return;
    case AMD64_SHR16:
        os << "shrw";
        return;
    case AMD64_SHR32:
        os << "shrl";
        return;
    case AMD64_SHR64:
        os << "shrq";
        return;
    case AMD64_SAR8:
        os << "sarb";
        return;
    case AMD64_SAR16:
        os << "sarw";
        return;
    case AMD64_SAR32:
        os << "sarl";
        return;
    case AMD64_SAR64:
        os << "sarq";
        return;
    case AMD64_CMP8:
        os << "cmpb";
        return;
    case AMD64_CMP16:
        os << "cmpw";
        return;
    case AMD64_CMP32:
        os << "cmpl";
        return;
    case AMD64_CMP64:
        os << "cmpq";
        return;
    case AMD64_NOT8:
        os << "notb";
        return;
    case AMD64_NOT16:
        os << "notw";
        return;
    case AMD64_NOT32:
        os << "notl";
        return;
    case AMD64_NOT64:
        os << "notq";
        return;
    case AMD64_NEG8:
        os << "negb";
        return;
    case AMD64_NEG16:
        os << "negw";
        return;
    case AMD64_NEG32:
        os << "negl";
        return;
    case AMD64_NEG64:
        os << "negq";
        return;
    case AMD64_JE:
        os << "je";
        return;
    case AMD64_JNE:
        os << "jne";
        return;
    case AMD64_JZ:
        os << "jz";
        return;
    case AMD64_JNZ:
        os << "jnz";
        return;
    case AMD64_JL:
        os << "jl";
        return;
    case AMD64_JLE:
        os << "jle";
        return;
    case AMD64_JG:
        os << "jg";
        return;
    case AMD64_JGE:
        os << "jge";
        return;
    case AMD64_JA:
        os << "ja";
        return;
    case AMD64_JAE:
        os << "jae";
        return;
    case AMD64_JB:
        os << "jb";
        return;
    case AMD64_JBE:
        os << "jbe";
        return;
    case AMD64_SETE:
        os << "sete";
        return;
    case AMD64_SETNE:
        os << "setne";
        return;
    case AMD64_SETZ:
        os << "setz";
        return;
    case AMD64_SETNZ:
        os << "setnz";
        return;
    case AMD64_SETL:
        os << "setl";
        return;
    case AMD64_SETLE:
        os << "setle";
        return;
    case AMD64_SETG:
        os << "setg";
        return;
    case AMD64_SETGE:
        os << "setge";
        return;
    case AMD64_SETA:
        os << "seta";
        return;
    case AMD64_SETAE:
        os << "setae";
        return;
    case AMD64_SETB:
        os << "setb";
        return;
    case AMD64_SETBE:
        os << "setbe";
        return;
    case AMD64_MOVSS:
        os << "movss";
        return;
    case AMD64_MOVSD:
        os << "movsd";
        return;
    case AMD64_MOVAPS:
        os << "movaps";
        return;
    case AMD64_MOVAPD:
        os << "movapd";
        return;
    case AMD64_UCOMISS:
        os << "ucomiss";
        return;
    case AMD64_UCOMISD:
        os << "ucomisd";
        return;
    case AMD64_ADDSS:
        os << "addss";
        return;
    case AMD64_ADDSD:
        os << "addsd";
        return;
    case AMD64_SUBSS:
        os << "subss";
        return;
    case AMD64_SUBSD:
        os << "subsd";
        return;
    case AMD64_MULSS:
        os << "mulss";
        return;
    case AMD64_MULSD:
        os << "mulsd";
        return;
    case AMD64_DIVSS:
        os << "divss";
        return;
    case AMD64_DIVSD:
        os << "divsd";
        return;
    case AMD64_ANDPS:
        os << "andps";
        return;
    case AMD64_ANDPD:
        os << "andpd";
        return;
    case AMD64_ORPS:
        os << "orps";
        return;
    case AMD64_ORPD:
        os << "orpd";
        return;
    case AMD64_XORPS:
        os << "xorps";
        return;
    case AMD64_XORPD:
        os << "xorpd";
        return;
    case AMD64_CVTSS2SD:
        os << "cvtss2sd";
        return;
    case AMD64_CVTSD2SS:
        os << "cvtsd2ss";
        return;
    case AMD64_CVTSI2SS:
        os << "cvtsi2ss";
        return;
    case AMD64_CVTSI2SD:
        os << "cvtsi2sd";
        return;
    case AMD64_VCVTUSI2SS:
        os << "vcvtusi2ss";
        return;
    case AMD64_VCVTUSI2SD:
        os << "vcvtusi2sd";
        return;
    case AMD64_CVTTSS2SI8:
        os << "cvttss2sib";
        return;
    case AMD64_CVTTSS2SI16:
        os << "cvttss2siw";
        return;
    case AMD64_CVTTSS2SI32:
        os << "cvttss2sil";
        return;
    case AMD64_CVTTSS2SI64:
        os << "cvttss2siq";
        return;
    case AMD64_CVTTSD2SI8:
        os << "cvttsd2sib";
        return;
    case AMD64_CVTTSD2SI16:
        os << "cvttsd2siw";
        return;
    case AMD64_CVTTSD2SI32:
        os << "cvttsd2sil";
        return;
    case AMD64_CVTTSD2SI64:
        os << "cvttsd2siq";
        return;
    case AMD64_VCVCTSS2USI8:
        os << "vcvctss2usib";
        return;
    case AMD64_VCVCTSS2USI16:
        os << "vcvctss2usiw";
        return;
    case AMD64_VCVCTSS2USI32:
        os << "vcvctss2usil";
        return;
    case AMD64_VCVCTSS2USI64:
        os << "vcvctss2usiq";
        return;
    case AMD64_VCVCTSD2USI8:
        os << "vcvctsd2usib";
        return;
    case AMD64_VCVCTSD2USI16:
        os << "vcvctsd2usiw";
        return;
    case AMD64_VCVCTSD2USI32:
        os << "vcvctsd2usil";
        return;
    case AMD64_VCVCTSD2USI64:
        os << "vcvctsd2usiq";
        return;
    }
}

void AsmWriter::writeRegister(std::ostream& os, AMD64_Register reg, uint8_t subreg) {
    os << '%';

    switch (reg) 
    {
    case RAX:
        switch (subreg)
        {
        case 1:
            os << "al";
            return;
        case 2:
            os << "ax";
            return;
        case 4:
            os << "eax";
            return;
        default:
            os << "rax";
            return;
        }

    case RBX:
        switch (subreg) 
        {
            case 1: 
                os << "bl";
                return;
            case 2: 
                os << "bx";
                return;
            case 4: 
                os << "ebx";
                return;
            default: 
                os << "rbx";
                return;
        }

    case RCX:
        switch (subreg) 
        {
            case 1:
                os << "cl";
                return;
            case 2: 
                os << "cx";
                return;
            case 4: 
                os << "ecx";
                return;
            default: 
                os << "rcx";
                return;
        }

    case RDX:
        switch (subreg) 
        {
            case 1: 
                os << "dl";
                return;
            case 2: 
                os << "dx";
                return;
            case 4: 
                os << "edx";
                return;
            default: 
                os << "rdx";
                return;
        }

    case RDI:
        switch (subreg) 
        {
            case 1: 
                os << "dil";
                return;
            case 2: 
                os << "di";
                return;
            case 4: 
                os << "edi";
                return;
            default: 
                os << "rdi";
                return;
        }

    case RSI:
        switch (subreg) 
        {
            case 1: 
                os << "sil";
                return;
            case 2: 
                os << "si";
                return;
            case 4: 
                os << "esi";
                return;
            default: 
                os << "rsi";
                return;
        }

    case RBP:
        switch (subreg) 
        {
            case 1: 
                os << "bpl";
                return;
            case 2: 
                os << "bp";
                return;
            case 4: 
                os << "ebp";
                return;
            default: 
                os << "rbp";
                return;
        }

    case RSP:
        switch (subreg) 
        {
            case 1: 
                os << "spl";
                return;
            case 2: 
                os << "sp";
                return;
            case 4: 
                os << "esp";
                return;
            default: 
                os << "rsp";
                return;
        }

    case R8:
        switch (subreg) 
        {
            case 1: 
                os << "r8b";
                return;
            case 2: 
                os << "r8w";
                return;
            case 4: 
                os << "r8d";
                return;
            default: 
                os << "r8";
                return;
        }

    case R9:
        switch (subreg) 
        {
            case 1: 
                os << "r9b";
                return;
            case 2: 
                os << "r9w";
                return;
            case 4: 
                os << "r9d";
                return;
            default: 
                os << "r9";
                return;
        }

    case R10:
        switch (subreg) 
        {
            case 1: 
                os << "r10b";
                return;
            case 2: 
                os << "r10w";
                return;
            case 4: 
                os << "r10d";
                return;
            default: 
                os << "r10";
                return;
        }

    case R11:
        switch (subreg) 
        {
            case 1: 
                os << "r11b";
                return;
            case 2: 
                os << "r11w";
                return;
            case 4: 
                os << "r11d";
                return;
            default: 
                os << "r11";
                return;
        }

    case R12:
        switch (subreg) 
        {
            case 1: 
                os << "r12b";
                return;
            case 2: 
                os << "r12w";
                return;
            case 4: 
                os << "r12d";
                return;
            default: 
                os << "r12";
                return;
        }

    case R13:
        switch (subreg) 
        {
            case 1: 
                os << "r13b";
                return;
            case 2: 
                os << "r13w";
                return;
            case 4: 
                os << "r13d";
                return;
            default: 
                os << "r13";
                return;
        }

    case R14:
        switch (subreg) 
        {
            case 1: 
                os << "r14b";
                return;
            case 2: 
                os << "r14w";
                return;
            case 4: 
                os << "r14d";
                return;
            default: 
                os << "r14";
                return;
        }

    case R15:
        switch (subreg) 
        {
            case 1: 
                os << "r15b";
                return;
            case 2: 
                os << "r15w";
                return;
            case 4: 
                os << "r15d";
                return;
            default: 
                os << "r15";
                return;
        }

    case RIP:
        os << "rip";
        return;

    case XMM0:
        os << "xmm0";
        return;
        
    case XMM1:
        os << "xmm1";
        return;
        
    case XMM2:
        os << "xmm2";
        return;

    case XMM3: 
        os << "xmm3";
        return;

    case XMM4: 
        os << "xmm4";
        return;

    case XMM5: 
        os << "xmm5";
        return;
        
    case XMM6: 
        os << "xmm6";
        return;

    case XMM7: 
        os << "xmm7";
        return;

    case XMM8: 
        os << "xmm8";
        return;

    case XMM9: 
        os << "xmm9";
        return;

    case XMM10:
        os << "xmm10";
        return;

    case XMM11:
        os << "xmm11";
        return;

    case XMM12:
        os << "xmm12";
        return;

    case XMM13:
        os << "xmm13";
        return;

    case XMM14:
        os << "xmm14";
        return;

    case XMM15:
        os << "xmm15";
        return;

    default:
        assert(false && "invalid AMD64 register!");
    }
}

void AsmWriter::writeOperand(std::ostream& os, const MachineOperand& operand) {
    switch (operand.kind()) 
    {
        case MachineOperand::Kind::Register: {
            assert(operand.reg().reg().is_physical() && "(1) cannot emit non-phyiscal register!");

            const MachineRegister& reg = operand.reg();

            writeRegister(os, static_cast<AMD64_Register>(reg.reg().id()), reg.subreg());
            return;
        }

        case MachineOperand::Kind::Memory: {
            const Memory& mem = operand.mem();
            if (mem.offset != 0)
                os << mem.offset;

            const MachineRegister& base = mem.base;

            assert(base.reg().is_physical() && "(2) cannot emit non-phyiscal register!");

            os << '(';
            writeRegister(os, static_cast<AMD64_Register>(base.reg().id()), base.subreg());
            os << ')';

            return;
        }

        case MachineOperand::Kind::Immediate:
            os << std::format("${}", operand.imm());
            return;

        case MachineOperand::Kind::Data: {
            const MachineData* data = operand.data();

            if (data->hasPool()) {
                const ConstantPool* pool = data->pool();
                os << std::format(".LFC{}_{}(%rip)", m_ids[pool->get_parent()], operand.data()->name());
            } else {
                os << std::format("{}(%rip)", operand.data()->name());
            }
            
            return;
        }

        case MachineOperand::Kind::Local: {
            const MachineLocal* local = operand.local();

            os << -local->get_offset() - static_cast<int32_t>(local->get_size());

            os << "(%rbp)";
            return;
        }

        case MachineOperand::Kind::Function:
            os << operand.function()->get_name();
            return;

        case MachineOperand::Kind::Label:
            os << std::format(".LBB{}_{}", m_ids.at(m_func), operand.label()->position());
            return;
    }
}

void AsmWriter::writeOp(std::ostream& os, const MachineOp& op) {
    if (op.has_comment())
        os << std::format("#\t{}", op.get_comment());

    if (op.is_intrinsic()) {
        switch (static_cast<Intrinsic>(op.op())) 
        {
        case Intrinsic::Husk:
        case Intrinsic::Param:
            return;

        case Intrinsic::Stack_Setup:
            os << "\tpushq\t%rbp\n\tmovq\t%rsp, %rbp\n";
            return;

        case Intrinsic::Stack_Reserve: {
            const StackFrame& frame = m_func->get_stack_frame();
            const uint32_t size = frame.size();
            if (size != 0)
                os << std::format("\tsubq\t${}, %rsp\n", size);

            return;
        }

        case Intrinsic::Stack_Restore:
            os << "\tmovq\t%rbp, %rsp\n\tpopq\t%rbp\n";
            return;
            
        case Intrinsic::Callsite_Set:
            os << "#\tCALLSITE_SET\n";
            return;

        case Intrinsic::Callsite_End:
            os << "#\tCALLSITE_END\n";
            return;
        }
    }

    os << '\t';
    writeOpcode(os, static_cast<AMD64_Op>(op.op()));
    os << '\t';

    for (uint32_t i = 0, e = op.num_explicit_operands(); i < e; ) {
        writeOperand(os, op.get_operand(i));

        if (++i != e)
            os << ", ";
    }

    os << '\n';
}

void AsmWriter::writeLabel(std::ostream& os, const MachineLabel& label) {
    os << std::format(".LBB{}_{}:\n", m_ids.at(m_func), label.position());

    const MachineOp* op = label.get_head();
    while (op) {
        writeOp(os, *op);
        op = op->get_next();
    }
}

void AsmWriter::writeFunction(std::ostream& os, const MachineFunction& func) {
    m_ids.emplace(&func, m_ids.size());
    m_func = &func;

    const ConstantPool& pool = func.get_pool();
    for (const MachineData* data : pool.get_constants()) {
        writeData(os, *data);
    }

    const std::string& name = func.get_name();

    if (func.isGlobal())
        os << std::format("\t.global\t{}\n", name);

    os << std::format("\t.text\n\t.type\t{}, @function\n{}:\n", name, name);
    
    for (uint32_t i = 0; i < func.num_labels(); ++i)
        writeLabel(os, *func.get_label(i));

    os << std::format(".LFE{}:\n\t.size\t{}, .-{}\n\n", m_ids.at(&func), name, name);

    m_func = nullptr;
}

void AsmWriter::writeConstant(std::ostream& os, const MachineConstant& constant) {
    switch (constant.kind()) 
    {
    case MachineConstant::Kind::Zero:
        os << std::format("\t.zero {}\n", constant.get_zeros());
        break;

    case MachineConstant::Kind::Int8:
        os << std::format("\t.byte {}\n", constant.get_int());
        break;

    case MachineConstant::Kind::Int16:
        os << std::format("\t.word {}\n", constant.get_int());
        break;

    case MachineConstant::Kind::Int32:
        os << std::format("\t.long {}\n", constant.get_int());
        break;

    case MachineConstant::Kind::Int64:
        os << std::format("\t.quad {}\n", constant.get_int());
        break;

    case MachineConstant::Kind::Float32: {
        const float value = constant.get_fp();
        os << std::format("#\t{:.5f}\n\t.long 0x", value);

        uint32_t bits = 0;
        std::memcpy(&bits, &value, sizeof(bits));
        os << std::hex << bits << std::dec << '\n';
        break;
    }

    case MachineConstant::Kind::Float64:
        const double value = constant.get_fp();
        os << std::format("#\t{:.5f}\n\t.quad 0x", value);

        uint64_t bits = 0;
        std::memcpy(&bits, &value, sizeof(bits));
        os << std::hex << bits << std::dec << '\n';
        break;
    }
}

void AsmWriter::writeData(std::ostream& os, const MachineData& data) {
    if (data.isReadonly()) {
        os << "\t.section\t.rodata\n";
    } else {
        os << "\t.data\n";
    }

    std::string name = data.name();
    if (data.hasPool()) {
        const ConstantPool* pool = data.pool();
        name = std::format(".LFC{}_{}", m_ids[pool->get_parent()], name);
    }

    if (data.isGlobal())
        os << std::format("\t.global{}\n", name);

    os << std::format("\t.type\t{}, @object\n{}:\n", name, name);

    for (const MachineConstant& constant : data.data())
        writeConstant(os, constant);
}
