#include "siir/basicblock.hpp"
#include "siir/machine_register.hpp"
#include "siir/machine_function.hpp"
#include "siir/machine_basicblock.hpp"
#include "siir/machine_operand.hpp"
#include "siir/machine_inst.hpp"
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
    for (auto curr = m_function->front(); curr; curr = curr->next()) {
        const auto* BB = curr->get_basic_block();
        m_insert = curr;

        for (const auto* inst = BB->front(); inst; inst = inst->next())
            select(inst);
    }
}

/*
MachineRegister X64InstSelection::mcreg(u32 vreg) {
    auto it = m_vregs.find(vreg);
    if (it != m_vregs.end())
        return it->second;

    VRegInfo info;
    info.start = 0;
    info.end = 0;
    switch (width) {
    case WD_None:
    case WD_Byte:
    case WD_Short:
    case WD_Half:
    case WD_Word:
        info.cls = GeneralPurpose;
        break;
    case WD_Single:
    case WD_Double:
        info.cls = FloatingPoint;
        break;
    }

    FunctionRegisterInfo& regi = m_function->get_register_info();
    u32 next_id = regi.vregs.size() + MachineRegister::VirtualBarrier;

    m_function->get_register_info().vregs.emplace(next_id, info);
    m_vregs.emplace(vreg, next_id);

    return next_id;
}
*/

void X64InstSelection::select(const Instruction* inst) {
    switch (inst->opcode()) {
    case INST_OP_NOP:
    case INST_OP_CONSTANT:
    case INST_OP_STRING:
    case INST_OP_LOAD:
    case INST_OP_STORE:
    case INST_OP_ACCESS_PTR:
    case INST_OP_SELECT:
    case INST_OP_BRANCH_IF:
    case INST_OP_JUMP:
    case INST_OP_PHI:
    case INST_OP_RETURN:
    case INST_OP_ABORT:
    case INST_OP_UNREACHABLE:
    case INST_OP_CALL:
    case INST_OP_IADD:
    case INST_OP_FADD:
    case INST_OP_ISUB:
    case INST_OP_FSUB:
    case INST_OP_SMUL:
    case INST_OP_UMUL:
    case INST_OP_FMUL:
    case INST_OP_SDIV:
    case INST_OP_UDIV:
    case INST_OP_FDIV:
    case INST_OP_SREM:
    case INST_OP_UREM:
    case INST_OP_FREM:
    case INST_OP_AND:
    case INST_OP_OR:
    case INST_OP_XOR:
    case INST_OP_SHL:
    case INST_OP_SHR:
    case INST_OP_SAR:
    case INST_OP_NOT:
    case INST_OP_INEG:
    case INST_OP_FNEG:
    case INST_OP_SEXT:
    case INST_OP_ZEXT:
    case INST_OP_FEXT:
    case INST_OP_ITRUNC:
    case INST_OP_FTRUNC:
    case INST_OP_SI2FP:
    case INST_OP_UI2FP:
    case INST_OP_FP2SI:
    case INST_OP_FP2UI:
    case INST_OP_P2I:
    case INST_OP_I2P:
    case INST_OP_REINTERPET:
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
        break;
    }
}
