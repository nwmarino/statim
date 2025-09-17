#include "siir/cfg.hpp"
#include "x64/x64.hpp"
#include <iomanip>

using namespace stm;
using namespace stm::siir;
using namespace stm::siir::x64;

static void emit_newline(std::ostream& os) {
    os << '\n';
}

static void emit_label(std::ostream& os, const std::string& label) {
    os << label << ":\n";
}

static void emit_numeric(std::ostream& os, i64 value) {
    os << value;
}

static void emit_register(std::ostream& os, MachineRegister reg, u16 subreg) {
    assert(reg.is_physical() && "cannot emit virtual register!");

    os << '%' << x64::to_string(static_cast<x64::Register>(reg.id()), subreg);
}

static void emit_immediate(std::ostream& os, i64 imm) {
    os << '$' << imm;
}

static void emit_symbol(std::ostream& os, const std::string& symbol) {
    os << symbol;
}

static void emit_opc(std::ostream& os, x64::Opcode opc) {
    os << "    ";

    switch (opc) {
    case NOP:
        os << "nop";
        break;
    case LEA:
        os << "leaq";
        break;
    case CALL:
        os << "call";
        break;
    case RET:
        os << "ret";
        break;
    case JMP:
        os << "j";
        break;
    case MOV:
        os << "mov";
        break;
    case UD2:
        os << "ud2";
        break;
    case CQO:
        os << "cqo";
        break;
    case PUSH64:
        os << "pushq";
        break;
    case POP64:
        os << "popq";
        break;
    case MOV8:
        os << "movb";
        break;
    case MOV16:
        os << "movw";
        break;
    case MOV32:
        os << "movl";
        break;
    case MOV64:
        os << "movq";
        break;
    case ADD8:
        os << "addb";
        break;
    case ADD16:
        os << "addw";
        break;
    case ADD32:
        os << "addl";
        break;
    case ADD64:
        os << "addq";
        break;
    case SUB8:
        os << "subb";
        break;
    case SUB16:
        os << "subw";
        break;
    case SUB32:
        os << "subl";
        break;
    case SUB64:
        os << "subq";
        break;
    case MUL8:
        os << "mulb";
        break;
    case MUL16:
        os << "mulw";
        break;
    case MUL32:
        os << "mull";
        break;
    case MUL64:
        os << "mulq";
        break;
    case IMUL8:
        os << "imulb";
        break;
    case IMUL16:
        os << "imulw";
        break;
    case IMUL32:
        os << "imull";
        break;
    case IMUL64:
        os << "imulq";
        break;
    case DIV8:
        os << "divb";
        break;
    case DIV16:
        os << "divw";
        break;
    case DIV32:
        os << "divl";
        break;
    case DIV64:
        os << "divq";
        break;
    case IDIV8:
        os << "idivb";
        break;
    case IDIV16:
        os << "idivw";
        break;
    case IDIV32:
        os << "idivl";
        break;
    case IDIV64:
        os << "idivq";
        break;
    case AND8:
        os << "andb";
        break;
    case AND16:
        os << "andw";
        break;
    case AND32:
        os << "andl";
        break;
    case AND64:
        os << "andq";
        break;
    case OR8:
        os << "orb";
        break;
    case OR16:
        os << "orw";
        break;
    case OR32:
        os << "orl";
        break;
    case OR64:
        os << "orq";
        break;
    case XOR8:
        os << "xorb";
        break;
    case XOR16:
        os << "xorw";
        break;
    case XOR32:
        os << "xorl";
        break;
    case XOR64:
        os << "xorq";
        break;
    case SHL8:
        os << "shlb";
        break;
    case SHL16:
        os << "shlw";
        break;
    case SHL32:
        os << "shll";
        break;
    case SHL64:
        os << "shlq";
        break;
    case SHR8:
        os << "shrb";
        break;
    case SHR16:
        os << "shrw";
        break;
    case SHR32:
        os << "shrl";
        break;
    case SHR64:
        os << "shrq";
        break;
    case SAR8:
        os << "sarb";
        break;
    case SAR16:
        os << "sarw";
        break;
    case SAR32:
        os << "sarl";
        break;
    case SAR64:
        os << "sarq";
        break;
    case CMP8:
        os << "cmpb";
        break;
    case CMP16:
        os << "cmpw";
        break;
    case CMP32:
        os << "cmpl";
        break;
    case CMP64:
        os << "cmpq";
        break;
    case NOT8:
        os << "notb";
        break;
    case NOT16:
        os << "notw";
        break;
    case NOT32:
        os << "notl";
        break;
    case NOT64:
        os << "notq";
        break;
    case NEG8:
        os << "negb";
        break;
    case NEG16:
        os << "negw";
        break;
    case NEG32:
        os << "negl";
        break;
    case NEG64:
        os << "negq";
        break;
    case MOVABS:
        os << "movabs";
        break;
    case MOVSX:
        os << "movsx";
        break;
    case MOVSXD:
        os << "movsxd";
        break;
    case MOVZX:
        os << "movzx";
        break;
    case MOVZXD:
        os << "movzxd";
        break;
    case JE:
        os << "je";
        break;
    case JNE:
        os << "jne";
        break;
    case JZ:
        os << "jz";
        break;
    case JNZ:
        os << "jnz";
        break;
    case JL:
        os << "jl";
        break;
    case JLE:
        os << "jle";
        break;
    case JG:
        os << "jg";
        break;
    case JGE:
        os << "jge";
        break;
    case JA:
        os << "ja";
        break;
    case JAE:
        os << "jae";
        break;
    case JB:
        os << "jb";
        break;
    case JBE:
        os << "jbe";
        break;
    case SETE:
        os << "sete";
        break;
    case SETNE:
        os << "setne";
        break;
    case SETZ:
        os << "setz";
        break;
    case SETNZ:
        os << "setnz";
        break;
    case SETL:
        os << "setl";
        break;
    case SETLE:
        os << "setle";
        break;
    case SETG:
        os << "setg";
        break;
    case SETGE:
        os << "setge";
        break;
    case SETA:
        os << "seta";
        break;
    case SETAE:
        os << "setae";
        break;
    case SETB:
        os << "setb";
        break;
    case SETBE:
        os << "setbe";
        break;
    case MOVSS:
        os << "movss";
        break;
    case MOVSD:
        os << "movsd";
        break;
    case MOVAPS:
        os << "movaps";
        break;
    case MOVAPD:
        os << "movapd";
        break;
    case UCOMISS:
        os << "ucomiss";
        break;
    case UCOMISD:
        os << "ucomisd";
        break;
    case ADDSS:
        os << "addss";
        break;
    case ADDSD:
        os << "addsd";
        break;
    case SUBSS:
        os << "subss";
        break;
    case SUBSD:
        os << "subsd";
        break;
    case MULSS:
        os << "mulss";
        break;
    case MULSD:
        os << "mulsd";
        break;
    case DIVSS:
        os << "divss";
        break;
    case DIVSD:
        os << "divsd";
        break;
    case ANDPS:
        os << "andps";
        break;
    case ANDPD:
        os << "andpd";
        break;
    case ORPS:
        os << "orps";
        break;
    case ORPD:
        os << "orpd";
        break;
    case XORPS:
        os << "xorps";
        break;
    case XORPD:
        os << "xorpd";
        break;
    case CVTSS2SD:
        os << "cvtss2sd";
        break;
    case CVTSD2SS:
        os << "cvtsd2ss";
        break;
    case CVTSI2SS:
        os << "cvtsi2ss";
        break;
    case CVTSI2SD:
        os << "cvtsi2sd";
        break;
    case CVTTSS2SI8:
        os << "cvttss2sib";
        break;
    case CVTTSS2SI16:
        os << "cvtss2siw";
        break;
    case CVTTSS2SI32:
        os << "cvtss2sil";
        break;
    case CVTTSS2SI64:
        os << "cvtss2siq";
        break;
    case CVTTSD2SI8:
        os << "cvtsd2sib";
        break;
    case CVTTSD2SI16:
        os << "cvtsd2siw";
        break;
    case CVTTSD2SI32:
        os << "cvtsd2sil";
        break;
    case CVTTSD2SI64:
        os << "cvtsd2siq";
        break;
    default:
        assert(false && "unrecognized x64 opcode!");
    }
}

static void write_operand(std::ostream& os, const MachineFunction& MF,
                          const MachineOperand& MO) {
    switch (MO.kind()) {
    case MachineOperand::MO_Register: {
        MachineRegister reg = MO.get_reg();
        if (reg.is_virtual())
            reg = MF.get_register_info().vregs.at(reg.id()).alloc;

        emit_register(os, reg, MO.get_subreg());
        break;
    }

    case MachineOperand::MO_Memory: {
        emit_numeric(os, MO.get_mem_disp());

        os << '(';

        MachineRegister reg = MO.get_mem_base();
        if (reg.is_virtual())
            reg = MF.get_register_info().vregs.at(reg.id()).alloc;

        emit_register(os, reg, 8);
        os << ')';
        break;
    }

    case MachineOperand::MO_Immediate:
        emit_immediate(os, MO.get_imm());
        break;

    case MachineOperand::MO_StackIdx: {
        const FunctionStackInfo& stack = MF.get_stack_info();
        const FunctionStackEntry& slot = stack.entries.at(MO.get_stack_index());

        i64 offset = -slot.offset - (i32) slot.size;
        emit_numeric(os, offset);

        os << '(';
        emit_register(os, x64::RBP, 8);
        os << ')';
        break;
    }

    case MachineOperand::MO_BasicBlock: {
        os << ".LBB" << MO.get_mmb()->position();
        break;
    }

    case MachineOperand::MO_ConstantIdx: {
        const FunctionConstantPool& cpool = MF.get_constant_pool();
        const FunctionConstantPoolEntry& constant = 
            cpool.entries.at(MO.get_constant_index());

        os << ".LCPI" << MO.get_constant_index() << "(%%rip)";
        break;
    }

    case MachineOperand::MO_Symbol: {
        emit_symbol(os, MO.get_symbol());
        break;
    }

    }
}

static void write_instruction(std::ostream& os, const MachineFunction& MF,
                              const MachineInst& MI) {
    emit_opc(os, static_cast<x64::Opcode>(MI.opcode()));

    os << "    ";

    for (u32 idx = 0, e = MI.num_explicit_operands(); idx != e; ++idx) {
        const MachineOperand& MO = MI.get_operand(idx);        
        write_operand(os, MF, MO);
        if (idx + 1 != e)
            os << ", ";
    }

    if (x64::is_call_opcode(static_cast<x64::Opcode>(MI.opcode())))
        os << "@PLT";

    emit_newline(os);
}

static void write_basic_block(std::ostream& os, const MachineFunction& MF, 
                              const MachineBasicBlock& MBB) {
    emit_label(os, ".LBB" + std::to_string(MBB.position()));

    for (auto& MI : MBB.insts()) {
        write_instruction(os, MF, MI);
    }
}

static void write_function(std::ostream& os, const MachineFunction& MF) {
    emit_label(os, MF.get_name());

    for (const auto* MBB = MF.front(); MBB; MBB = MBB->next()) {
        write_basic_block(os, MF, *MBB);
    }
}

void X64AsmWriter::run(std::ostream& os) const {
    for (const auto& global : m_obj.get_graph()->globals()) {

    }

    for (const auto& [name, function] : m_obj.functions()) {
        write_function(os, *function);
    }
}
