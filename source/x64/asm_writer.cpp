#include "siir/cfg.hpp"
#include "siir/constant.hpp"
#include "siir/machine_function.hpp"
#include "x64/x64.hpp"

#include <cmath>

using namespace stm;
using namespace stm::siir;
using namespace stm::siir::x64;

static u32 g_function_id = 0;

static void emit_register(std::ostream& os, MachineRegister reg, u16 subreg) {
    assert(reg.is_physical() && "cannot emit virtual register!");

    os << '%' << x64::to_string(static_cast<x64::Register>(reg.id()), subreg);
}

static const char* opc_as_string(x64::Opcode opc) {
    switch (opc) {
    case NOP:         return "nop";
    case LEA:         return "leaq";
    case CALL:        return "callq";
    case RET:         return "retq";
    case JMP:         return "j";
    case MOV:         return "mov";
    case UD2:         return "ud2";
    case CQO:         return "cqo";
    case PUSH64:      return "pushq";
    case POP64:       return "popq";
    case MOV8:        return "movb";
    case MOV16:       return "movw";
    case MOV32:       return "movl";
    case MOV64:       return "movq";
    case ADD8:        return "addb";
    case ADD16:       return "addw";
    case ADD32:       return "addl";
    case ADD64:       return "addq";
    case SUB8:        return "subb";
    case SUB16:       return "subw";
    case SUB32:       return "subl";
    case SUB64:       return "subq";
    case MUL8:        return "mulb";
    case MUL16:       return "mulw";
    case MUL32:       return "mull";
    case MUL64:       return "mulq";
    case IMUL8:       return "imulb";
    case IMUL16:      return "imulw";
    case IMUL32:      return "imull";
    case IMUL64:      return "imulq";
    case DIV8:        return "divb";
    case DIV16:       return "divw";
    case DIV32:       return "divl";
    case DIV64:       return "divq";
    case IDIV8:       return "idivb";
    case IDIV16:      return "idivw";
    case IDIV32:      return "idivl";
    case IDIV64:      return "idivq";
    case AND8:        return "andb";
    case AND16:       return "andw";
    case AND32:       return "andl";
    case AND64:       return "andq";
    case OR8:         return "orb";
    case OR16:        return "orw";
    case OR32:        return "orl";
    case OR64:        return "orq";
    case XOR8:        return "xorb";
    case XOR16:       return "xorw";
    case XOR32:       return "xorl";
    case XOR64:       return "xorq";
    case SHL8:        return "shlb";
    case SHL16:       return "shlw";
    case SHL32:       return "shll";
    case SHL64:       return "shlq";
    case SHR8:        return "shrb";
    case SHR16:       return "shrw";
    case SHR32:       return "shrl";
    case SHR64:       return "shrq";
    case SAR8:        return "sarb";
    case SAR16:       return "sarw";
    case SAR32:       return "sarl";
    case SAR64:       return "sarq";
    case CMP8:        return "cmpb";
    case CMP16:       return "cmpw";
    case CMP32:       return "cmpl";
    case CMP64:       return "cmpq";
    case NOT8:        return "notb";
    case NOT16:       return "notw";
    case NOT32:       return "notl";
    case NOT64:       return "notq";
    case NEG8:        return "negb";
    case NEG16:       return "negw";
    case NEG32:       return "negl";
    case NEG64:       return "negq";
    case MOVABS:      return "movabs";
    case MOVSX:       return "movsx";
    case MOVSXD:      return "movsxd";
    case MOVZX:       return "movzx";
    case MOVZXD:      return "movzxd";
    case JE:          return "je";
    case JNE:         return "jne";
    case JZ:          return "jz";
    case JNZ:         return "jnz";
    case JL:          return "jl";
    case JLE:         return "jle";
    case JG:          return "jg";
    case JGE:         return "jge";
    case JA:          return "ja";
    case JAE:         return "jae";
    case JB:          return "jb";
    case JBE:         return "jbe";
    case SETE:        return "sete";
    case SETNE:       return "setne";
    case SETZ:        return "setz";
    case SETNZ:       return "setnz";
    case SETL:        return "setl";
    case SETLE:       return "setle";
    case SETG:        return "setg";
    case SETGE:       return "setge";
    case SETA:        return "seta";
    case SETAE:       return "setae";
    case SETB:        return "setb";
    case SETBE:       return "setbe";
    case MOVSS:       return "movss";
    case MOVSD:       return "movsd";
    case MOVAPS:      return "movaps";
    case MOVAPD:      return "movapd";
    case UCOMISS:     return "ucomiss";
    case UCOMISD:     return "ucomisd";
    case ADDSS:       return "addss";
    case ADDSD:       return "addsd";
    case SUBSS:       return "subss";
    case SUBSD:       return "subsd";
    case MULSS:       return "mulss";
    case MULSD:       return "mulsd";
    case DIVSS:       return "divss";
    case DIVSD:       return "divsd";
    case ANDPS:       return "andps";
    case ANDPD:       return "andpd";
    case ORPS:        return "orps";
    case ORPD:        return "orpd";
    case XORPS:       return "xorps";
    case XORPD:       return "xorpd";
    case CVTSS2SD:    return "cvtss2sd";
    case CVTSD2SS:    return "cvtsd2ss";
    case CVTSI2SS:    return "cvtsi2ss";
    case CVTSI2SD:    return "cvtsi2sd";
    case CVTTSS2SI8:  return "cvttss2sib";
    case CVTTSS2SI16: return "cvtss2siw";
    case CVTTSS2SI32: return "cvtss2sil";
    case CVTTSS2SI64: return "cvtss2siq";
    case CVTTSD2SI8:  return "cvtsd2sib";
    case CVTTSD2SI16: return "cvtsd2siw";
    case CVTTSD2SI32: return "cvtsd2sil";
    case CVTTSD2SI64: return "cvtsd2siq";
    default:
        assert(false && "unrecognized x64 opcode!");
    }
}

static void emit_operand(std::ostream& os, const MachineFunction& MF,
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
        os << MO.get_mem_disp() << '(';

        MachineRegister reg = MO.get_mem_base();
        if (reg.is_virtual())
            reg = MF.get_register_info().vregs.at(reg.id()).alloc;

        emit_register(os, reg, 8);
        os << ')';
        break;
    }

    case MachineOperand::MO_Immediate:
        os << '$' << MO.get_imm();
        break;

    case MachineOperand::MO_StackIdx: {
        const FunctionStackInfo& stack = MF.get_stack_info();
        const FunctionStackEntry& slot = stack.entries.at(MO.get_stack_index());

        os << -slot.offset - (i32) slot.size << '(';
        emit_register(os, x64::RBP, 8);
        os << ')';
        break;
    }

    case MachineOperand::MO_BasicBlock: {
        os << ".LBB" << g_function_id << '_' << MO.get_mmb()->position();
        break;
    }

    case MachineOperand::MO_ConstantIdx: {
        const FunctionConstantPool& cpool = MF.get_constant_pool();
        const FunctionConstantPoolEntry& constant = 
            cpool.entries.at(MO.get_constant_index());

        os << ".LCPI" << g_function_id << '_' << MO.get_constant_index() << 
            "(%rip)";
        break;
    }

    case MachineOperand::MO_Symbol: {
        os << MO.get_symbol();
        break;
    }

    default:
        assert(false && "unrecognized machine operand kind for x64!");
    }
}

static void emit_instruction(std::ostream& os, const MachineFunction& MF,
                             const MachineInst& MI) {
    if (is_ret_opcode(static_cast<x64::Opcode>(MI.opcode()))) {
        os << "\taddq\t$" << MF.get_stack_info().alignment() << ", %rsp\n"
           << "\tpopq\t%rbp\n"
           << "\t.cfi_def_cfa %rsp, 8\n"
           << "\tretq\n";
           
        return;
    }

    os << '\t' << opc_as_string(static_cast<x64::Opcode>(MI.opcode())) << '\t';

    for (u32 idx = 0, e = MI.num_explicit_operands(); idx != e; ++idx) {
        const MachineOperand& MO = MI.get_operand(idx);        
        emit_operand(os, MF, MO);
        if (idx + 1 != e)
            os << ", ";
    }

    if (x64::is_call_opcode(static_cast<x64::Opcode>(MI.opcode())))
        os << "@PLT";

    os << '\n';
}

static void emit_basic_block(std::ostream& os, const MachineFunction& MF, 
                              const MachineBasicBlock& MBB) {
    if (MBB.get_basic_block()->is_entry_block() || 
      !MBB.get_basic_block()->has_preds()) {
        os << "#bb" << MBB.position() << ":\n";
    } else {
        os << ".LBB" << g_function_id << '_' << MBB.position() << ":\n";
    }

    for (auto& MI : MBB.insts()) {
        emit_instruction(os, MF, MI);
    }
}

static void emit_constant(std::ostream& os, const MachineFunction& MF,
                          const Constant* constant, u32 size) {
    os << "\t.";

    if (auto CI = dynamic_cast<const ConstantInt*>(constant)) {
        switch (size) {
        case 1:
            os << "byte ";
            break;
        case 2:
            os << "word ";
            break;
        case 4:
            os << "long ";
            break;
        case 8:
            os << "quad ";
            break;
        }

        os << CI->get_value();
    } else if (auto CFP = dynamic_cast<const ConstantFP*>(constant)) {
        switch (size) {
        case 4: {
            os << "long 0x";

            u32 bits;
            f32 value = CFP->get_value();
            std::memcpy(&bits, &value, sizeof(bits));
            os << std::hex << bits << std::dec;
            break;
        }

        case 8: {
            os << "quad 0x";

            u64 bits;
            f64 value = CFP->get_value();
            std::memcpy(&bits, &value, sizeof(bits));
            os << std::hex << bits << std::dec;
            break;
        }

        default:
            assert(false && "unsupported SSE floating point size!");
        }
    } else if (auto CN = dynamic_cast<const ConstantNull*>(constant)) {
        os << "quad 0x0";
    } else if (auto CS = dynamic_cast<const ConstantString*>(constant)) {
        os << "string \"" << CS->get_value() << "\"";
    }

    os << '\n';
}

static void emit_function(std::ostream& os, const MachineFunction& MF) {
    os << "# begin function " << MF.get_name() << '\n';

    const FunctionConstantPool& cpool = MF.get_constant_pool();
    i32 last_size = -1;
    for (u32 idx = 0, e = cpool.num_entries(); idx != e; ++idx) {
        const FunctionConstantPoolEntry& entry = cpool.entries.at(idx);
        const Constant* constant = entry.constant;

        u32 size = MF.get_target().get_type_size(constant->get_type());
        if (size != last_size) {
            os << "\t.section\t.rodata.cst" << size << ",\"aM\",@progbits,8\n"
               << "\t.p2align\t" << std::log2(size) << ", 0x0\n";

            last_size = size;
        }

        os << ".LCPI" << g_function_id << '_' << idx << ":\n";
        emit_constant(os, MF, entry.constant, size);
    }

    os << "\t.text\n";

    if (MF.get_function()->get_linkage() == Function::LINKAGE_EXTERNAL)
        os << "\t.global\t" << MF.get_name() << '\n';

    os << "\t.p2align 4\n"
       << "\t.type\t" << MF.get_name() << ", @function\n"
       << MF.get_name() << ":\n"
       << "\t.cfi_startproc\n"
       << "\tpushq\t%rbp\n"
       << "\t.cfi_def_cfa_offset 16\n"
       << "\t.cfi_offset %rbp, -16\n"
       << "\tmovq\t%rsp, %rbp\n"
       << "\t.cfi_def_cfa_register %rbp\n"
       << "\tsubq\t$" << MF.get_stack_info().alignment() << ", %rsp\n";

    for (const auto* MBB = MF.front(); MBB; MBB = MBB->next()) {
        emit_basic_block(os, MF, *MBB);
    }

    os << ".LFE" << g_function_id << ":\n"
       << "\t.size\t" << MF.get_name() << ", .LFE" << g_function_id << "-" << MF.get_name() << '\n'
       << "\t.cfi_endproc\n"
       << "# end function " << MF.get_name() << "\n\n";
}

void X64AsmWriter::run(std::ostream& os) const {
    g_function_id = 0;

    os << "\t.file\t\"" << m_obj.get_graph()->get_file().filename() << "\"\n";

    for (const auto& global : m_obj.get_graph()->globals()) {
        /// TODO: Implement global emits.
    }

    for (const auto& [name, function] : m_obj.functions()) {
        emit_function(os, *function);
        g_function_id++;
    }

    os << "\t.ident\t\t\"stmc: 1.0.0, nwmarino\"\n" 
       << "\t.section\t.note.GNU-stack,\"\",@progbits\n";
}
