#include "siir/basicblock.hpp"
#include "siir/constant.hpp"
#include "siir/function.hpp"
#include "siir/global.hpp"
#include "siir/inlineasm.hpp"
#include "siir/instruction.hpp"
#include "siir/local.hpp"
#include "siir/machine_register.hpp"
#include "siir/machine_function.hpp"
#include "siir/machine_basicblock.hpp"
#include "siir/machine_operand.hpp"
#include "siir/machine_inst.hpp"
#include "siir/type.hpp"
#include "x64/x64.hpp"

#include <cassert>

using namespace stm;
using namespace stm::siir;
using namespace stm::siir::x64;

/// Flip the conditional jump opcode |jcc| operand-wise. This is different from
/// negating the operation.
static x64::Opcode flip_jcc(x64::Opcode jcc) {
    switch (jcc) {
    case JE:
    case JNE:
    case JZ:
    case JNZ:
        return jcc;
    case JL:
        return JG;
    case JLE:
        return JGE;
    case JG:
        return JL;
    case JGE:
        return JLE;
    case JA:
        return JB;
    case JAE:
        return JBE;
    case JB:
        return JA;
    case JBE:
        return JAE;
    default:
        assert(false && "cannot flip non-jcc opcode!");
    }
}

/// Negate the conditional jump opcode |jcc|, retaining signedness.
static x64::Opcode negate_jcc(x64::Opcode jcc) {
    switch (jcc) {
    case JE:
        return JNE;
    case JNE:
        return JE;
    case JZ:
        return JNZ;
    case JNZ:
        return JE;
    case JL:
        return JGE;
    case JLE:
        return JG;
    case JG:
        return JLE;
    case JGE:
        return JL;
    case JA:
        return JBE;
    case JAE:
        return JB;
    case JB:
        return JAE;
    case JBE:
        return JA;
    default:
        assert(false && "cannot negate non-jcc opcode!");
    }
}

/// Flip the conditional set opcode |setcc| operand-wise. This is different 
/// from negating the operation.
static x64::Opcode flip_setcc(x64::Opcode setcc) {
    switch (setcc) {
    case SETE:
    case SETNE:
    case SETZ:
    case SETNZ:
        return setcc;
    case SETL:
        return SETG;
    case SETLE:
        return SETGE;
    case SETG:
        return SETL;
    case SETGE:
        return SETLE;
    case SETA:
        return SETB;
    case SETAE:
        return SETBE;
    case SETB:
        return SETA;
    case SETBE:
        return SETAE;
    default:
        assert(false && "cannot flip non-setcc opcode!");
    }
}

/// Negate the conditional set opcode |setcc|, retaining signedness.
static x64::Opcode negate_setcc(x64::Opcode setcc) {
    switch (setcc) {
    case SETE:
        return SETNE;
    case SETNE:
        return SETE;
    case SETZ:
        return SETNZ;
    case SETNZ:
        return SETZ;
    case SETL:
        return SETGE;
    case SETLE:
        return SETG;
    case SETG:
        return SETLE;
    case SETGE:
        return SETL;
    case SETA:
        return SETBE;
    case SETAE:
        return SETB;
    case SETB:
        return SETAE;
    case SETBE:
        return SETA;
    default:
        assert(false && "cannot negate non-setcc opcode!");
    }
}

void X64InstSelection::run() {
    FunctionStackInfo& frame = m_function->get_stack_info();
    u32 stack_index = 0, stack_offset = 0;
    for (const auto& [name, local] : m_function->get_function()->locals()) {
        FunctionStackEntry entry;
        entry.offset = stack_offset;

        u32 size = m_target.get_type_size(local->get_allocated_type());
        stack_offset += size;

        entry.size = size;
        entry.align = m_target.get_type_align(local->get_allocated_type());
        entry.local = local;

        frame.entries.push_back(entry);
        m_stack_indices.emplace(local, stack_index++);
    }

    for (auto* curr = m_function->front(); curr; curr = curr->next()) {
        const auto* BB = curr->get_basic_block();
        m_insert = curr;

        for (const auto* inst = BB->front(); inst; inst = inst->next())
            select(inst);
    }
}

MachineRegister X64InstSelection::as_machine_reg(const Instruction* inst) {
    assert(inst->result_id() != 0 && "instruction does not produce a value!");

    VRegInfo info;
    info.cls = GeneralPurpose;
    info.alloc = MachineRegister::NoRegister;

    if (inst->get_type()->is_floating_point_type())
        info.cls = FloatingPoint;

    FunctionRegisterInfo& regi = m_function->get_register_info();
    u32 next_id = regi.vregs.size() + MachineRegister::VirtualBarrier;
    regi.vregs.emplace(next_id, info);
    m_vregs.emplace(inst->result_id(), next_id);
    return next_id;
}

u16 X64InstSelection::get_subreg(const Type* ty) const {
    if (!ty)
        return 0;

    u32 size = m_target.get_type_size(ty);
    assert(1 <= size && size <= 8 && 
        "cannot determine width for a non-scalar type!");

    return size;
}

x64::Opcode X64InstSelection::get_move_op(const Type* ty) const {
    assert(ty && "type cannot be null!");

    u32 size = m_target.get_type_size_in_bits(ty);
    switch (size) {
    case 1:
    case 8:
        return x64::MOV8;
    case 16:
        return x64::MOV16;
    case 32:
        if (ty->is_floating_point_type())
            return x64::MOVSS;
        else
            return x64::MOV32;
    case 64:
        if (ty->is_floating_point_type())
            return x64::MOVSD;
        else
            return x64::MOV64;
    }

    assert(false && "cannot determine move opcode based on type!");
}

x64::Opcode X64InstSelection::get_cmp_op(const Type* ty) const {
    assert(ty && "type cannot be null!");

    u32 size = m_target.get_type_size_in_bits(ty);
    switch (size) {
    case 1:
    case 8:
        return x64::CMP8;
    case 16:
        return x64::CMP16;
    case 32:
        if (ty->is_floating_point_type())
            return x64::UCOMISS;
        else
            return x64::CMP32;
    case 64:
        if (ty->is_floating_point_type())
            return x64::UCOMISD;
        else
            return x64::CMP64;
    }

    assert(false && "cannot determine cmp opcode based on type!");
}

x64::Opcode X64InstSelection::get_add_op(const Type* ty) const {
    assert(ty && "type cannot be null!");

    u32 size = m_target.get_type_size_in_bits(ty);
    switch (size) {
    case 1:
    case 8:
        return x64::ADD8;
    case 16:
        return x64::ADD16;
    case 32:
        if (ty->is_floating_point_type())
            return x64::ADDSS;
        else
            return x64::ADD32;
    case 64:
        if (ty->is_floating_point_type())
            return x64::ADDSD;
        else
            return x64::ADD64;
    }

    assert(false && "cannot determine add opcode based on type!");
}

x64::Opcode X64InstSelection::get_sub_op(const Type* ty) const {
    assert(ty && "type cannot be null!");

    u32 size = m_target.get_type_size_in_bits(ty);
    switch (size) {
    case 1:
    case 8:
        return x64::SUB8;
    case 16:
        return x64::SUB16;
    case 32:
        if (ty->is_floating_point_type())
            return x64::SUBSS;
        else
            return x64::SUB32;
    case 64:
        if (ty->is_floating_point_type())
            return x64::SUBSD;
        else
            return x64::SUB64;
    }

    assert(false && "cannot determine sub opcode based on type!");
}

x64::Opcode X64InstSelection::get_and_op(const Type* ty) const {
    assert(ty && "type cannot be null!");

    u32 size = m_target.get_type_size_in_bits(ty);
    switch (size) {
    case 1:
    case 8:
        return x64::AND8;
    case 16:
        return x64::AND16;
    case 32:
        if (ty->is_floating_point_type())
            return x64::ANDPS;
        else
            return x64::AND32;
    case 64:
        if (ty->is_floating_point_type())
            return x64::ANDPD;
        else
            return x64::AND64;
    }

    assert(false && "cannot determine and opcode based on type!");
}

x64::Opcode X64InstSelection::get_or_op(const Type* ty) const {
    assert(ty && "type cannot be null!");

    u32 size = m_target.get_type_size_in_bits(ty);
    switch (size) {
    case 1:
    case 8:
        return x64::OR8;
    case 16:
        return x64::OR16;
    case 32:
        if (ty->is_floating_point_type())
            return x64::ORPS;
        else
            return x64::OR32;
    case 64:
        if (ty->is_floating_point_type())
            return x64::ORPD;
        else
            return x64::OR64;
    }

    assert(false && "cannot determine or opcode based on type!");
}

x64::Opcode X64InstSelection::get_xor_op(const Type* ty) const {
    assert(ty && "type cannot be null!");

    u32 size = m_target.get_type_size_in_bits(ty);
    switch (size) {
    case 1:
    case 8:
        return x64::XOR8;
    case 16:
        return x64::XOR16;
    case 32:
        if (ty->is_floating_point_type())
            return x64::XORPS;
        else
            return x64::XOR32;
    case 64:
        if (ty->is_floating_point_type())
            return x64::XORPD;
        else
            return x64::XOR64;
    }

    assert(false && "cannot determine xor opcode based on type!");
}

x64::Opcode X64InstSelection::get_not_op(const Type* ty) const {
    assert(ty && "type cannot be null!");

    u32 size = m_target.get_type_size_in_bits(ty);
    switch (size) {
    case 1:
    case 8:
        return x64::NOT8;
    case 16:
        return x64::NOT16;
    case 32:
        return x64::NOT32;
    case 64:
        return x64::NOT64;
    }

    assert(false && "cannot determine not opcode based on type!");
}

x64::Opcode X64InstSelection::get_neg_op(const Type* ty) const {
        assert(ty && "type cannot be null!");

    u32 size = m_target.get_type_size_in_bits(ty);
    switch (size) {
    case 1:
    case 8:
        return x64::NEG8;
    case 16:
        return x64::NEG16;
    case 32:
        return x64::NEG32;
    case 64:
        return x64::NEG64;
    }

    assert(false && "cannot determine neg opcode based on type!");
}

x64::Opcode X64InstSelection::get_jcc_op(siir::Opcode opc) const {
    switch (opc) {
    case INST_OP_CMP_IEQ:
        return x64::JE;
    case INST_OP_CMP_INE:
        return x64::JNE;
    case INST_OP_CMP_OEQ:
        return x64::JE;
    case INST_OP_CMP_ONE:
        return x64::JNE;
    case INST_OP_CMP_UNEQ:
        return x64::JE;
    case INST_OP_CMP_UNNE:
        return x64::JNE;
    case INST_OP_CMP_SLT:
        return x64::JL;
    case INST_OP_CMP_SLE:
        return x64::JLE;
    case INST_OP_CMP_SGT:
        return x64::JG;
    case INST_OP_CMP_SGE:
        return x64::JGE;
    case INST_OP_CMP_ULT:
        return x64::JB;
    case INST_OP_CMP_ULE:
        return x64::JBE;
    case INST_OP_CMP_UGT:
        return x64::JA;
    case INST_OP_CMP_UGE:
        return x64::JAE;
    case INST_OP_CMP_OLT:
        return x64::JB;
    case INST_OP_CMP_OLE:
        return x64::JBE;
    case INST_OP_CMP_OGT:
        return x64::JA;
    case INST_OP_CMP_OGE:
        return x64::JAE;
    case INST_OP_CMP_UNLT:
        return x64::JB;
    case INST_OP_CMP_UNLE:
        return x64::JBE;
    case INST_OP_CMP_UNGT:
        return x64::JA;
    case INST_OP_CMP_UNGE:
        return x64::JAE;
    default:
        assert(false && "expected comparison opcode!");
    }
}

x64::Opcode X64InstSelection::get_setcc_op(siir::Opcode opc) const {
    switch (opc) {
    case INST_OP_CMP_IEQ:
        return x64::SETE;
    case INST_OP_CMP_INE:
        return x64::SETNE;
    case INST_OP_CMP_OEQ:
        return x64::SETE;
    case INST_OP_CMP_ONE:
        return x64::SETNE;
    case INST_OP_CMP_UNEQ:
        return x64::SETE;
    case INST_OP_CMP_UNNE:
        return x64::SETNE;
    case INST_OP_CMP_SLT:
        return x64::SETL;
    case INST_OP_CMP_SLE:
        return x64::SETLE;
    case INST_OP_CMP_SGT:
        return x64::SETG;
    case INST_OP_CMP_SGE:
        return x64::SETGE;
    case INST_OP_CMP_ULT:
        return x64::SETB;
    case INST_OP_CMP_ULE:
        return x64::SETBE;
    case INST_OP_CMP_UGT:
        return x64::SETA;
    case INST_OP_CMP_UGE:
        return x64::SETAE;
    case INST_OP_CMP_OLT:
        return x64::SETB;
    case INST_OP_CMP_OLE:
        return x64::SETBE;
    case INST_OP_CMP_OGT:
        return x64::SETA;
    case INST_OP_CMP_OGE:
        return x64::SETAE;
    case INST_OP_CMP_UNLT:
        return x64::SETB;
    case INST_OP_CMP_UNLE:
        return x64::SETBE;
    case INST_OP_CMP_UNGT:
        return x64::SETA;
    case INST_OP_CMP_UNGE:
        return x64::SETAE;
    default:
        assert(false && "expected comparison opcode!");
    }
}

MachineOperand X64InstSelection::as_operand(const Value* value) const {
    if (auto CI = dynamic_cast<const ConstantInt*>(value)) {
        return MachineOperand::create_imm(CI->get_value());
    } else if (auto CFP = dynamic_cast<const ConstantFP*>(value)) {
        return MachineOperand::create_constant_index(
            m_function->get_constant_pool().get_or_create_constant(
                CFP, m_target.get_type_align(value->get_type())));
    } else if (auto CN = dynamic_cast<const ConstantNull*>(value)) {
        return MachineOperand::create_imm(0);
    } else if (auto CBA = dynamic_cast<const BlockAddress*>(value)) {
        /// TODO: Rewrite to accomodate for possible positional changes in 
        /// machine blocks.
        return MachineOperand::create_block(
            m_function->at(CBA->get_block()->get_number()));
    } else if (auto CGL = dynamic_cast<const Global*>(value)) {
        return MachineOperand::create_symbol(CGL->get_name().c_str());
    } else if (auto ARG = dynamic_cast<const Argument*>(value)) {
        return as_call_argument(ARG, ARG->get_number());
    } else if (auto FN = dynamic_cast<const Function*>(value)) {
        return MachineOperand::create_symbol(FN->get_name().c_str());
    } else if (auto LCL = dynamic_cast<const Local*>(value)) {
        return MachineOperand::create_stack_index(m_stack_indices.at(LCL));
    } else if (auto IN = dynamic_cast<const Instruction*>(value)) {
        /// TODO: Remove with instruction selection completion, without this,
        /// opcodes that are not yet implemented won't ever produce a virtual
        /// register mapping.
        if (m_vregs.count(IN->result_id()) == 0)
            return MachineOperand::create_imm(0);

        return MachineOperand::create_reg(
            m_vregs.at(IN->result_id()), 
            get_subreg(IN->get_type()), 
            false);
    }

    assert(false && "cannot lower value to machine operand!");
}

MachineOperand X64InstSelection::as_call_argument(
        const Value* value, u32 arg_idx) const {

    if (arg_idx < 6) {
        MachineRegister reg;

        if (value->get_type()->is_floating_point_type()) {
            switch (arg_idx) {
            case 0:
                reg = x64::XMM0;
                break;
            case 1:
                reg = x64::XMM1;
                break;
            case 2:
                reg = x64::XMM2;
                break;
            case 3:
                reg = x64::XMM3;
                break;
            case 4:
                reg = x64::XMM4;
                break;
            case 5:
                reg = x64::XMM5;
                break;
            }
        } else {
            switch (arg_idx) {
            case 0:
                reg = x64::RDI;
                break;
            case 1:
                reg = x64::RSI;
                break;
            case 2:
                reg = x64::RDX;
                break;
            case 3:
                reg = x64::RCX;
                break;
            case 4:
                reg = x64::R8;
                break;
            case 5:
                reg = x64::R9;
                break;
            }
        }

        return MachineOperand::create_reg(
            reg, get_subreg(value->get_type()), true);
    }

    assert(false && "calls with more than 6 arguments not implemented!");
}

MachineInst& X64InstSelection::emit(x64::Opcode op, 
                                    const std::vector<MachineOperand>& ops) {
    assert(m_insert && "insertion block not set!");
    MachineInst inst { op, ops, m_insert };
    return m_insert->back();
}

void X64InstSelection::select(const Instruction* inst) {
    switch (inst->opcode()) {
    case INST_OP_NOP:
        emit(x64::NOP);
        break;

    case INST_OP_JUMP:
        emit(x64::JMP, { as_operand(inst->get_operand(0)) });
        break;

    case INST_OP_ABORT:
        emit(x64::UD2);
        break;

    case INST_OP_UNREACHABLE:
        break;

    case INST_OP_CONSTANT:
        select_constant(inst);
        break;

    case INST_OP_STRING:
        select_string_constant(inst);
        break;

    case INST_OP_LOAD:
    case INST_OP_STORE:
        select_load_store(inst);
        break;

    case INST_OP_ACCESS_PTR:
        select_access_ptr(inst);
        break;

    case INST_OP_SELECT:
        select_select(inst);
        break;

    case INST_OP_BRANCH_IF:
        select_branch_if(inst);
        break;

    case INST_OP_PHI:
        select_phi(inst);
        break;

    case INST_OP_RETURN:
        select_return(inst);
        break;

    case INST_OP_CALL:
        select_call(inst);
        break;

    case INST_OP_IADD:
    case INST_OP_FADD:
        select_add(inst);
        break;

    case INST_OP_ISUB:
    case INST_OP_FSUB:
        select_sub(inst);
        break;

    case INST_OP_SMUL:
    case INST_OP_UMUL:
    case INST_OP_FMUL:
    case INST_OP_SDIV:
    case INST_OP_UDIV:
    case INST_OP_FDIV:
    case INST_OP_SREM:
    case INST_OP_UREM:
    case INST_OP_FREM:
        select_mul_div_rem(inst);
        break;

    case INST_OP_AND:
    case INST_OP_OR:
    case INST_OP_XOR:
        select_bit_op(inst);
        break;

    case INST_OP_SHL:
    case INST_OP_SHR:
    case INST_OP_SAR:
        select_shift(inst);
        break;

    case INST_OP_NOT:
        select_not(inst);
        break;

    case INST_OP_INEG:
    case INST_OP_FNEG:
        select_neg(inst);
        break;

    case INST_OP_SEXT:
    case INST_OP_ZEXT:
    case INST_OP_FEXT:
        select_ext(inst);
        break;

    case INST_OP_ITRUNC:
    case INST_OP_FTRUNC:
        select_trunc(inst);
        break;

    case INST_OP_SI2FP:
    case INST_OP_UI2FP:
        select_int_to_fp_cvt(inst);
        break;

    case INST_OP_FP2SI:
    case INST_OP_FP2UI:
        select_fp_to_int_cvt(inst);
        break;

    case INST_OP_P2I:
        select_ptr_to_int_cvt(inst);
        break;

    case INST_OP_I2P:
        select_int_to_ptr_cvt(inst);
        break;

    case INST_OP_REINTERPET:
        select_type_reinterpret(inst);
        break;

    case INST_OP_CMP_IEQ:
    case INST_OP_CMP_INE:
    case INST_OP_CMP_OEQ:
    case INST_OP_CMP_ONE:
    case INST_OP_CMP_UNEQ:
    case INST_OP_CMP_UNNE:
    case INST_OP_CMP_SLT:
    case INST_OP_CMP_SLE:
    case INST_OP_CMP_SGT:
    case INST_OP_CMP_SGE:
    case INST_OP_CMP_ULT:
    case INST_OP_CMP_ULE:
    case INST_OP_CMP_UGT:
    case INST_OP_CMP_UGE:
    case INST_OP_CMP_OLT:
    case INST_OP_CMP_OLE:
    case INST_OP_CMP_OGT:
    case INST_OP_CMP_OGE:
    case INST_OP_CMP_UNLT:
    case INST_OP_CMP_UNLE:
    case INST_OP_CMP_UNGT:
    case INST_OP_CMP_UNGE:
        select_comparison(inst);
        break;
    }
}

void X64InstSelection::select_constant(const Instruction* inst) {
    assert(inst->is_const() && "expected ConstInstr opcode!");

    const auto* value = static_cast<const Constant*>(
        inst->get_operand(0));

    MachineOperand src = as_operand(inst->get_operand(0));
    emit(get_move_op(inst->get_type()), { src })
        .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
}

void X64InstSelection::select_string_constant(const Instruction* inst) {
    assert(inst->opcode() == INST_OP_STRING && "expected StringInstr opcode!");

    const auto* string = static_cast<const ConstantString*>(
        inst->get_operand(0));
    u32 cpool_index = 
        m_function->get_constant_pool().get_or_create_constant(string, 1);

    emit(x64::LEA64)
        .add_constant_index(cpool_index)
        .add_reg(as_machine_reg(inst), 8, true);
}

void X64InstSelection::select_load_store(const Instruction* inst) {
    assert((inst->is_load() || inst->is_store()) &&
        "expected LoadInstr or StoreInstr opcode!");

    x64::Opcode opc = get_move_op(
        inst->is_load() ? inst->get_type() : inst->get_operand(0)->get_type());

    MachineOperand src = as_operand(inst->get_operand(0));
    if (inst->is_load() && src.is_reg()) {
        // The pointer to load from is in a register, e.g. the result of a
        // pointer access, so it must be transformed into a memory reference to
        // dereference the pointer.
        src = MachineOperand::create_mem(src.get_reg(), 0);

        if (src.get_mem_base().is_physical()) {
            src.set_is_use(true);

            if (dynamic_cast<const Argument*>(inst->get_operand(0)))
                src.set_is_kill(true);
        }
    }

    if (inst->is_store()) {
        if (src.is_reg() && src.get_reg().is_physical()) {
            src.set_is_use(true);

            if (dynamic_cast<const Argument*>(inst->get_operand(0)))
                src.set_is_kill(true);
        } else if (src.is_mem() || src.is_stack_index() || src.is_constant_index()) {
            // Both the store source and destination are memory references, so
            // the source must first be placed into a temporary register, we
            // choose %rax for simplicity.
            MachineOperand tmp = MachineOperand::create_reg(
                x64::RAX, 
                get_subreg(inst->get_operand(0)->get_type()), 
                true);

            emit(x64::LEA64, { src, tmp });

            // Now the source of the store can be considered tmp (in %rax), and
            // the next use will kill the value in it.
            src = tmp;
            src.set_is_use();
            src.set_is_kill();
        }

        MachineOperand dst = as_operand(inst->get_operand(1));
        if (dst.is_reg()) {
            // The pointer to store to is in a register, e.g. the result 
            // of a pointer access, so it must be transformed into a memory 
            // reference.
            dst = MachineOperand::create_mem(dst.get_reg(), 0);
            
            if (dst.get_mem_base().is_physical()) {
                dst.set_is_use(true);
            }                
        }

        emit(opc, { src, dst });
    } else {
        emit(opc, { src })
            .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
    }
}

void X64InstSelection::select_access_ptr(const Instruction* inst) {
    const Value* src_value = inst->get_operand(0);
    const Type* src_type = src_value->get_type();
    MachineOperand src = as_operand(src_value);
    MachineOperand dst = MachineOperand::create_reg(
        as_machine_reg(inst), 8, true);
    
    assert(src_type->is_pointer_type() &&
        "APInstr source must be a pointer!");
    const Type* pointee = static_cast<const PointerType*>(
        src_type)->get_pointee();

    x64::Opcode opc;
    if (dynamic_cast<const Local*>(src_value)) {
        opc = x64::LEA64;
    } else {
        opc = get_move_op(src_type);
    }

    emit(opc, { src, dst });

    i64 offset;
    if (auto constant = dynamic_cast<const ConstantInt*>(inst->get_operand(1))) {
        if (pointee->is_struct_type()) {
            offset = m_target.get_field_offset(
                static_cast<const StructType*>(pointee), 
                constant->get_value());
        } else {
            offset = m_target.get_type_size(pointee) * constant->get_value();
        }

        if (offset == 0)
            return;

        emit(x64::ADD64)
            .add_imm(offset)
            .add_operand(dst);
    } else {
        /// TODO: This is mangled from an old expirementation, needs work.
        switch (pointee->get_kind()) {
        case Type::TK_Array:
            offset = m_target.get_type_size(
                static_cast<const ArrayType*>(pointee)->get_element_type());
            break;
        case Type::TK_Function:
            offset = m_target.get_type_size(
                static_cast<const PointerType*>(pointee)->get_pointee());
            break;
        default:
            offset = m_target.get_type_size(pointee);
        }

        MachineOperand index = as_operand(inst->get_operand(1));
        MachineOperand multiplier = MachineOperand::create_imm(offset);
         
        if (offset == 1) {
            emit(x64::ADD64, { index, dst });
        } else {
            MachineOperand tmp = MachineOperand::create_reg(
                x64::RAX, 8, true);

            emit(x64::IMUL64)
                .add_imm(offset)
                .add_operand(index)
                .add_operand(tmp);

            tmp.set_is_use();
            tmp.set_is_kill();

            emit(x64::ADD64, { tmp, dst });
        }
    }
}

void X64InstSelection::select_select(const Instruction* inst) {
    assert(false && "ISEL not implemented!");
}

void X64InstSelection::select_branch_if(const Instruction* inst) {
    const Value* condition = inst->get_operand(0);
    assert(condition->get_type()->is_integer_type(1) &&
        "BranchIfInstr condition type is not 'i1'!");

    const auto* instr = dynamic_cast<const Instruction*>(condition);
    if (instr && instr->is_comparison() && is_deferred(instr)) {
        x64::Opcode jcc = get_jcc_op(instr->opcode());
        MachineOperand lhs = as_operand(instr->get_operand(0));
        MachineOperand rhs = as_operand(instr->get_operand(1));

        if (rhs.is_imm()) {
            MachineOperand tmp = lhs;
            lhs = rhs;
            rhs = tmp;
        } else {
            jcc = flip_jcc(jcc);
        }

        x64::Opcode cmp_opc = get_cmp_op(instr->get_operand(0)->get_type());
        emit(cmp_opc, { lhs, rhs });

        MachineOperand tdst = as_operand(inst->get_operand(1));
        MachineOperand fdst = as_operand(inst->get_operand(2));
        emit(jcc, { tdst });
        emit(x64::JMP, { fdst });
    } else {
        MachineOperand cond = as_operand(condition);
        MachineOperand tdst = as_operand(inst->get_operand(1));
        MachineOperand fdst = as_operand(inst->get_operand(2));
        MachineOperand zero = MachineOperand::create_imm(0);

        /// TODO: Adjust for floating point comparisons, using XORPx for zero.
        emit(x64::CMP8, { zero, cond });
        emit(x64::JNE, { tdst });
        emit(x64::JMP, { fdst });
    }
}

void X64InstSelection::select_phi(const Instruction* inst) {
    assert(false && "ISEL not implemented!");
}

void X64InstSelection::select_return(const Instruction* inst) {
    MachineRegister dst_reg = MachineRegister::NoRegister;
    u32 sub_reg = 0;

    if (inst->num_operands() == 1) {
        const Value* return_value = inst->get_operand(0);
        if (return_value->get_type()->is_floating_point_type()) {
            dst_reg = x64::XMM0;
        } else {
            dst_reg = x64::RAX;
            sub_reg = get_subreg(return_value->get_type());
        }

        MachineOperand src = as_operand(return_value);
        x64::Opcode opc = get_move_op(return_value->get_type());
        emit(opc, { src })
            .add_reg(dst_reg, sub_reg, false);
    }

    MachineInst& instr = emit(x64::RET64);

    if (dst_reg != MachineRegister::NoRegister)
        instr.add_reg(dst_reg, sub_reg, false, true);
}

void X64InstSelection::select_call(const Instruction* inst) {
    assert(inst->num_operands() <= 7 && 
        "cannot call a function with more than 6 arguments!");
    
    const Value* first_oper = inst->get_operand(0);
    if (const auto* iasm = dynamic_cast<const InlineAsm*>(first_oper)) {
        /// TODO: Implement inline assembly selection.
        return;
    }

    /// TODO: Add stack spilling for calls with more than 6 arguments.

    std::vector<MachineRegister> regs = {};
    regs.reserve(inst->num_operands() - 1);

    for (u32 idx = 0, e = inst->num_operands() - 1; idx != e; ++idx) {
        const Value* arg = inst->get_operand(idx + 1);
        MachineOperand src = as_operand(arg);
        MachineOperand dst = as_call_argument(arg, idx);
        dst.set_is_def(true);
        regs.push_back(dst.get_reg());

        x64::Opcode opc;
        if (dynamic_cast<const Local*>(arg)) {
            opc = x64::LEA64;
        } else {
            opc = get_move_op(arg->get_type());
        }

        emit(opc, { src, dst });
    }

    const Function* callee = dynamic_cast<const Function*>(first_oper);
    assert(callee && 
        "CallInstr first operand is not a function or inline assembly!");

    MachineInst& call = emit(x64::CALL64)
        .add_symbol(callee->get_name());
    
    for (const auto& reg : regs)
        call.add_reg(reg, 8, false, true, true);

    if (inst->result_id() != 0) {
        MachineRegister src_reg;
        u32 sub_reg = 0;
        if (inst->get_type()->is_floating_point_type()) {
            src_reg = x64::XMM0;
        } else {
            src_reg = x64::RAX;
            sub_reg = get_subreg(inst->get_type());
        }

        call.add_reg(src_reg, sub_reg, true, true);

        x64::Opcode opc = get_move_op(inst->get_type());
        emit(opc)
            .add_reg(src_reg, sub_reg, false, false, true, false)
            .add_reg(as_machine_reg(inst), sub_reg, true);
    }
}

void X64InstSelection::select_add(const Instruction* inst) {
    MachineOperand lhs = as_operand(inst->get_operand(0));
    MachineOperand rhs = as_operand(inst->get_operand(1));

    if (rhs.is_imm()) {
        MachineOperand tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }

    x64::Opcode add_opc = get_add_op(inst->get_type());
    MachineInst& instr = emit(add_opc, { lhs, rhs });

    //if (rhs.is_reg()) {
    //    instr.get_operand(1).set_is_kill();
    //    instr.add_reg(rhs.get_reg(), 8, true);
    //}

    x64::Opcode mov_opc = get_move_op(inst->get_type());
    emit(mov_opc, { rhs })
        .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
}

void X64InstSelection::select_sub(const Instruction* inst) {
    x64::Opcode sub_opc = get_sub_op(inst->get_type());
    x64::Opcode mov_opc = get_move_op(inst->get_type());
    MachineOperand lhs = as_operand(inst->get_operand(0));
    MachineOperand rhs = as_operand(inst->get_operand(1));

    if (lhs.is_imm()) {
        MachineOperand dst = MachineOperand::create_reg(
            as_machine_reg(inst), 
            get_subreg(inst->get_type()), 
            true);

        emit(mov_opc, { lhs, dst });
        emit(sub_opc, { rhs, dst });
    } else {
        emit(sub_opc, { rhs, lhs });
        emit(mov_opc, { lhs })
            .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
    }    
}

void X64InstSelection::select_mul_div_rem(const Instruction* inst) {
    assert(false && "ISEL not implemented!");
}

void X64InstSelection::select_bit_op(const Instruction* inst) {
    x64::Opcode opc;
    switch (inst->opcode()) {
    case INST_OP_AND:
        opc = get_and_op(inst->get_type());
        break;
    case INST_OP_OR:
        opc = get_or_op(inst->get_type());
        break;
    case INST_OP_XOR:
        opc = get_xor_op(inst->get_type());
        break;
    default:
        assert("expected AndInstr or OrInstr or XorInstr opcode!");
    }

    MachineOperand lhs = as_operand(inst->get_operand(0));
    MachineOperand rhs = as_operand(inst->get_operand(1));
    emit(opc, { lhs, rhs });

    x64::Opcode mov_opc = get_move_op(inst->get_type());
    emit(mov_opc, { rhs })
        .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
}

void X64InstSelection::select_shift(const Instruction* inst) {
    assert(false && "ISEL not implemented!");
}

void X64InstSelection::select_not(const Instruction* inst) {
    MachineOperand src = as_operand(inst->get_operand(0));
    emit(get_not_op(inst->get_operand(0)->get_type()), { src });

    emit(get_move_op(inst->get_type()), { src })
        .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
}

void X64InstSelection::select_neg(const Instruction* inst) {
    MachineOperand src = as_operand(inst->get_operand(0));

    if (inst->opcode() == INST_OP_INEG) {
        x64::Opcode opc = get_neg_op(inst->get_type());
        emit(opc, { src });

        x64::Opcode mov_opc = get_move_op(inst->get_type());
        emit(mov_opc, { src })
            .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
    } else if (inst->opcode() == INST_OP_FNEG) {
        /// TODO: Implement FNegInstr selection, also needs mask constants.
    } else {
        assert(false && "expected INegInstr or FNegInstr opcode!");
    }
}

void X64InstSelection::select_ext(const Instruction* inst) {
    const Value* value = inst->get_operand(0);
    MachineOperand src = as_operand(value);
    u32 src_sz_in_bits = m_target.get_type_size_in_bits(value->get_type());
    u32 dst_sz_in_bits = m_target.get_type_size_in_bits(inst->get_type());
    u32 dst_subreg = get_subreg(inst->get_type());
    x64::Opcode opc;

    switch (inst->opcode()) {
    case INST_OP_SEXT:
        if (src_sz_in_bits == 32 && dst_sz_in_bits == 64)
            opc = x64::MOVSXD;
        else
            opc = x64::MOVSX;

        break;

    case INST_OP_ZEXT:
        if (src_sz_in_bits == 32 && dst_sz_in_bits == 64) {
            opc = x64::MOV;
            dst_subreg = 4;
        } else {
            opc = x64::MOVZX;
        }

        break;
        
    case INST_OP_FEXT:
        opc = x64::CVTSS2SD;
        break;

    default:
        assert(false && 
            "expected SExtInstr or ZExtInstr or FExtInstr opcode!");
    }

    emit(opc, { src })
        .add_reg(as_machine_reg(inst), dst_subreg, true);
}

void X64InstSelection::select_trunc(const Instruction* inst) {
    MachineOperand src = as_operand(inst->get_operand(0));
    u32 dst_subreg = get_subreg(inst->get_type());
    x64::Opcode opc;

    switch (inst->opcode()) {
    case INST_OP_ITRUNC:
        if (src.is_reg())
            src.set_subreg(dst_subreg);
        
        opc = x64::MOV;
        break;

    case INST_OP_FTRUNC:
        opc = x64::CVTSD2SS;
        break;

    default:
        assert(false && 
            "expected ITruncInstr or FTruncInstr opcode!");
    }

    emit(opc, { src })
        .add_reg(as_machine_reg(inst), dst_subreg, true);
}

void X64InstSelection::select_int_to_fp_cvt(const Instruction* inst) {
    assert((inst->opcode() == INST_OP_SI2FP || inst->opcode() == INST_OP_UI2FP) 
        && "expected SI2FPInstr or UI2FPInstr opcode!");

    MachineOperand src = as_operand(inst->get_operand(0));
    const Type* dst_type = inst->get_type();
    x64::Opcode opc;

    if (dst_type->is_floating_point_type(32)) {
        opc = x64::CVTSI2SS;
    } else if (dst_type->is_floating_point_type(64)) {
        opc = x64::CVTSI2SD;
    } else {
        assert(false && "invalid integer to fp conversion destination type!");
    }

    emit(opc, { src })
        .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
}

void X64InstSelection::select_fp_to_int_cvt(const Instruction* inst) {
    assert((inst->opcode() == INST_OP_FP2SI || inst->opcode() == INST_OP_FP2UI) 
        && "expected FP2SIInstr or FP2UIInstr opcode!");

    const Value* operand = inst->get_operand(0);
    const Type* src_type = operand->get_type();
    u32 dst_size = m_target.get_type_size_in_bits(inst->get_type());
    MachineOperand src = as_operand(operand);
    x64::Opcode opc;

    if (src_type->is_floating_point_type(32)) {
        switch (dst_size) {
        case 8:
            opc = x64::CVTTSS2SI8;
            break;
        case 16:
            opc = x64::CVTTSS2SI16;
            break;
        case 32:
            opc = x64::CVTTSS2SI32;
            break;
        case 64:
            opc = x64::CVTTSS2SI64;
            break;
        }
    } else if (src_type->is_floating_point_type(64)) {
        switch (dst_size) {
        case 8:
            opc = x64::CVTTSD2SI8;
            break;
        case 16:
            opc = x64::CVTTSD2SI16;
            break;
        case 32:
            opc = x64::CVTTSD2SI32;
            break;
        case 64:
            opc = x64::CVTTSD2SI64;
            break;
        }
    } else {
        assert(false && "invalid fp to integer conversion destination type!");
    }

    emit(opc, { src })
        .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
}

void X64InstSelection::select_ptr_to_int_cvt(const Instruction* inst) {
    const Value* src = inst->get_operand(0);
    x64::Opcode opc;
    if (dynamic_cast<const Local*>(src)) {
        opc = x64::LEA64;
    } else {
        opc = get_move_op(src->get_type());
    }

    emit(opc)
        .add_operand(as_operand(src))
        .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
}

void X64InstSelection::select_int_to_ptr_cvt(const Instruction* inst) {
    emit(get_move_op(inst->get_type()))
        .add_operand(as_operand(inst->get_operand(0)))
        .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
}

void X64InstSelection::select_type_reinterpret(const Instruction* inst) {
    const Value* src = inst->get_operand(0);
    x64::Opcode opc;
    if (dynamic_cast<const Local*>(src)) {
        opc = x64::LEA64;
    } else {
        opc = get_move_op(src->get_type());
    }

    emit(opc)
        .add_operand(as_operand(src))
        .add_reg(as_machine_reg(inst), get_subreg(inst->get_type()), true);
}

void X64InstSelection::select_comparison(const Instruction* inst) {
    if (inst->num_uses() == 1) {
        // If the only user of this comparison is a conditional branch, then
        // we defer this instruction until later (at the location of the
        // branch) so that we can skip a set and subsequent comparison.
        const User* user = inst->use_front()->get_user();
        const auto* instr = dynamic_cast<const Instruction*>(user);
        if (instr && instr->is_branch_if()) {
            defer(inst);
            return;
        }
    }

    MachineOperand lhs = as_operand(inst->get_operand(0));
    MachineOperand rhs = as_operand(inst->get_operand(1));
    x64::Opcode cmp_opc = get_cmp_op(inst->get_operand(0)->get_type());
    emit(cmp_opc, { lhs, rhs });

    // Determine the conditional set mnemonic based on the comparison opcode.
    x64::Opcode setcc = get_setcc_op(inst->opcode());
    emit(setcc).add_reg(as_machine_reg(inst), 1, true);
}
