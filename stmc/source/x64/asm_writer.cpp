#include "siir/cfg.hpp"
#include "siir/constant.hpp"
#include "siir/global.hpp"
#include "siir/machine_function.hpp"
#include "siir/machine_operand.hpp"
#include "siir/machine_register.hpp"
#include "x64/x64.hpp"

#include <cmath>

using namespace stm;
using namespace stm::siir;
using namespace stm::siir::x64;

static u32 g_function_id = 0;

static const char* opc_as_string(x64::Opcode opc) {
    switch (opc) {
    case NOP:         return "nop";
    case JMP:         return "jmp";
    case UD2:         return "ud2";
    case CQO:         return "cqo";
    case MOV:         return "mov ";
    case CALL64:      return "callq";
    case RET64:       return "retq";
    case LEA32:       return "leal";
    case LEA64:       return "leaq";
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

/// Returns the mapped, physical register for |reg|. If |reg| is already a
/// physical register, this function does nothing.
static x64::Register map_register(MachineRegister reg, 
                                  const MachineFunction& MF) {
    if (reg.is_virtual())
        reg = MF.get_register_info().vregs.at(reg.id()).alloc;

    return static_cast<x64::Register>(reg.id());
}

/// Returns true if |MI| is a redundant `movx` instruction.
///
/// A move is considered redundant if both operands are identical physical 
/// registers.
static bool is_redundant_move(const MachineInst& MI, 
                              const MachineFunction& MF) {
    if (!is_move_opcode(static_cast<x64::Opcode>(MI.opcode())))
        return false;

    if (MI.num_operands() != 2)
        return false;

    const MachineOperand& oper1 = MI.get_operand(0);
    const MachineOperand& oper2 = MI.get_operand(1);

    if (!oper1.is_reg() || !oper2.is_reg())
        return false;

    x64::Register reg1 = map_register(oper1.get_reg(), MF);
    x64::Register reg2 = map_register(oper2.get_reg(), MF);
    return reg1 == reg2 && oper1.get_subreg() == oper2.get_subreg();
}

/// Emits the operand |MO| to output stream |os|.
static void emit_operand(std::ostream& os, const MachineFunction& MF,
                         const MachineOperand& MO) {
    switch (MO.kind()) {
    case MachineOperand::MO_Register: {
        MachineRegister reg = MO.get_reg();
        if (reg.is_virtual())
            reg = MF.get_register_info().vregs.at(reg.id()).alloc;

        os << '%' << to_string(map_register(MO.get_reg(), MF), MO.get_subreg());
        break;
    }

    case MachineOperand::MO_Memory: {
        if (MO.get_mem_disp() != 0)
            os << MO.get_mem_disp();

        os << "(%" << to_string(map_register(MO.get_mem_base(), MF), 8) << ')';
        break;
    }

    case MachineOperand::MO_Immediate:
        os << '$' << MO.get_imm();
        break;

    case MachineOperand::MO_StackIdx: {
        const FunctionStackInfo& stack = MF.get_stack_info();
        const FunctionStackEntry& slot = stack.entries.at(MO.get_stack_index());

        os << (-slot.offset - static_cast<i32>(slot.size)) << "(%rbp)";
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
    // Skip the emission of redundant moves.
    if (is_redundant_move(MI, MF))
        return;

    // If this is a return instruction, inject the necessary epilogue steps.
    if (is_ret_opcode(static_cast<x64::Opcode>(MI.opcode()))) {
        os << "\taddq\t$" << MF.get_stack_info().alignment() << ", %rsp\n"
           << "\tpopq\t%rbp\n"
           << "\t.cfi_def_cfa %rsp, 8\n"
           << "\tretq\n";
        return;
    }

    os << '\t' << opc_as_string(static_cast<x64::Opcode>(MI.opcode())) << '\t';

    // Emit all explicit operands of the instruction.
    for (u32 idx = 0, e = MI.num_explicit_operands(); idx != e; ++idx) {    
        emit_operand(os, MF, MI.get_operand(idx));

        if (idx + 1 != e)
            os << ", ";
    }

    if (x64::is_call_opcode(static_cast<x64::Opcode>(MI.opcode())))
        os << "@PLT";

    os << '\n';
}

static void emit_basic_block(std::ostream& os, const MachineFunction& MF, 
                              const MachineBasicBlock& MBB) {
    if (!MBB.get_basic_block()->has_preds()) {
        // For basic blocks without predecessors (usually only the entry block),
        // only emit a comment instead of the redundant label.
        os << "#bb" << MBB.position() << ":\n";
    } else {
        os << ".LBB" << g_function_id << '_' << MBB.position() << ":\n";
    }

    for (auto& MI : MBB.insts())
        emit_instruction(os, MF, MI);
}

static void emit_constant(std::ostream& os, const Target& target,
                          const Constant* constant) {
    u32 size = target.get_type_size(constant->get_type());

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
        os << "string \"";
        
        for (u32 idx = 0, e = CS->get_value().size(); idx != e; ++idx) {
            switch (CS->get_value()[idx]) {
            case '\\':
                os << "\\\\";
                break;
            case '\'':
                os << "\\'";
                break;
            case '\"':
                os << "\\\"";
                break;
            case '\n':
                os << "\\n";
                break;
            case '\t':
                os << "\\t";
                break;
            case '\r':
                os << "\\r";
                break;
            case '\b':
                os << "\\b";
                break;
            case '\0':
                os << "\\0";
                break;
            default:
                os << CS->get_value()[idx];
                break;
            }
        }

        os << '"';
    }

    os << '\n';
}

static void emit_function(std::ostream& os, const MachineFunction& MF) {
    const std::string& name = MF.get_name();

    os << "# begin function " << name << '\n';

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
        emit_constant(os, MF.get_target(), entry.constant);
    }

    os << "\t.text\n";

    if (MF.get_function()->get_linkage() == Function::LINKAGE_EXTERNAL)
        os << "\t.global\t" << name << '\n';

    os << "\t.p2align 4\n"
       << "\t.type\t" << name << ", @function\n"
       << name << ":\n"
       << "\t.cfi_startproc\n"
       << "\tpushq\t%rbp\n"
       << "\t.cfi_def_cfa_offset 16\n"
       << "\t.cfi_offset %rbp, -16\n"
       << "\tmovq\t%rsp, %rbp\n"
       << "\t.cfi_def_cfa_register %rbp\n"
       << "\tsubq\t$" << MF.get_stack_info().alignment() << ", %rsp\n";

    for (const auto* MBB = MF.front(); MBB; MBB = MBB->next())
        emit_basic_block(os, MF, *MBB);

    os << ".LFE" << g_function_id << ":\n"
       << "\t.size\t" << name << ", .LFE" << g_function_id << '-' << name << '\n'
       << "\t.cfi_endproc\n"
       << "# end function " << name << "\n\n";
}

static void emit_global(std::ostream& os, const Target& target, 
                        const Global* global) {
    if (global->is_read_only()) {
        os << "\t.section\t.rodata\n";
    } else {
        os << "\t.data\n";
    }

    if (global->get_linkage() == Global::LINKAGE_EXTERNAL)
        os << "\t.global " << global->get_name() << '\n';

    os << "\t.align\t" << target.get_type_align(global->get_initializer()->get_type()) << '\n'
       << "\t.type\t" << global->get_name() << ", @object\n"
       << "\t.size\t" << global->get_name() << ", " << target.get_type_size(global->get_initializer()->get_type()) << '\n'
       << global->get_name() << ":\n";

    emit_constant(os, target, global->get_initializer());
}

void X64AsmWriter::run(std::ostream& os) const {
    g_function_id = 0;

    os << "\t.file\t\"" << m_obj.get_graph()->get_file().filename() << "\"\n";

    for (const auto& global : m_obj.get_graph()->globals()) {
        emit_global(os, *m_obj.get_target(), global);
    }

    for (const auto& [name, function] : m_obj.functions()) {
        emit_function(os, *function);
        g_function_id++;
    }

    os << "\t.ident\t\t\"stmc: 1.0.0, nwmarino\"\n" 
       << "\t.section\t.note.GNU-stack,\"\",@progbits\n";
}
