#include "bytecode.hpp"
#include "target/amd64.hpp"
#include "machine/register.hpp"
#include "machine/function.hpp"
#include "machine/basicblock.hpp"
#include "machine/operand.hpp"
#include "machine/inst.hpp"

#include <cassert>

using namespace stm;
using namespace stm::amd64;

InstSelection::InstSelection(MachineFunction* function) 
    : m_function(function) {}

void InstSelection::run() {
    for (auto curr = m_function->front(); curr; curr = curr->next()) {
        const BasicBlock* BB = curr->get_basic_block();
        m_insert = curr;

        for (auto inst = BB->front(); inst; inst = inst->next())
            select(inst);
    }
}

amd64::Opcode InstSelection::flip_jcc(amd64::Opcode jcc) {
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
        assert(false && "non-jcc instruction");
    }
}

amd64::Opcode InstSelection::neg_jcc(amd64::Opcode jcc) {
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
        assert(false && "non-jcc instruction");
    }
}

amd64::Opcode InstSelection::flip_setcc(amd64::Opcode setcc) {
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
        assert(false && "non-setcc instruction");
        return NOP;
    }
}

amd64::Opcode InstSelection::neg_setcc(amd64::Opcode setcc) {
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
        assert(false && "non-setcc instruction");
        return NOP;
    }
}

MachineRegister InstSelection::mcreg(u32 vreg, Width width) {
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

MachineOperand InstSelection::lower(const Operand& operand) {
    switch (operand.get_type()) {
    case Operand::OP_Register: {
        if (operand.get_reg() == 0) {
            // v0 = stack, so return physical %RBP here.
            return MachineOperand::create_reg(amd64::Register::RBP, 0, false);
        } else {
            u16 subreg = 0;
            switch (operand.get_width()) {
            case WD_Byte:
                subreg = 8;
                break;
            case WD_Short:
                subreg = 16;
                break;
            case WD_Half:
                subreg = 32;
                break;
            case WD_Word:
                subreg = 64;
                break;
            default:
                break;
            }

            return MachineOperand::create_reg(
                mcreg(operand.get_reg(), operand.get_width()), 
                subreg, 
                false);
        }
    }
    
    case Operand::OP_Memory: {
        MachineRegister base;
        if (operand.get_mem_reg() == 0)
            base = amd64::Register::RBP;
        else
            base = mcreg(operand.get_mem_reg(), WD_Word);

        return MachineOperand::create_mem(base, operand.get_mem_disp());
    }

    case Operand::OP_Immediate:
        return MachineOperand::create_imm(operand.get_imm());

    case Operand::OP_FPImmediate: {
        /// TODO: Materialize floating points.
        return MachineOperand::create_imm(operand.get_imm());
    }

    case Operand::OP_Argument: {
        return MachineOperand::create_imm(operand.get_imm());
    }

    case Operand::OP_Return: {
        return MachineOperand::create_imm(operand.get_imm());
    }

    case Operand::OP_Block:
        return MachineOperand::create_block(
            m_function->at(operand.get_block()->get_number()));

    case Operand::OP_Function:
        return MachineOperand::create_symbol(
            operand.get_function()->get_name().data());        
    }
}

void InstSelection::select(Instruction* inst) {
    switch (inst->opcode()) {
    case Constant:
        select_const(inst);
        break;
    case Move:
        select_move(inst);
        break;
    case Lea:
        select_lea(inst);
        break;
    case Copy:
        select_copy(inst);
        break;
    case Jump:
        select_jump(inst);
        break;
    case BranchTrue:
    case BranchFalse:
        select_branch(inst);
        break;
    case SetTrue:
    case SetFalse:
        select_set(inst);
        break;
    case Return:
        select_return(inst);
        break;
    case Call:
        select_call(inst);
        break;
    case Add:
    case Sub:
    case Mul:
    case Div:
        select_arith(inst);
        break;
    case Inc:
    case Dec:
        select_crement(inst);
        break;
    case Neg:
        select_neg(inst);
        break;
    case Not:
        select_not(inst);
        break;
    case And:
    case Or:
    case Xor:
        select_logic(inst);
        break;
    case Shl:
    case Sar:
    case Shr:
        select_shift(inst);
        break;
    case SExt:
        select_sext(inst);
        break;
    case ZExt:
        select_zext(inst);
        break;
    case FExt:
        select_fext(inst);
        break;
    case Trunc:
        select_trunc(inst);
        break;
    case FTrunc:
        select_ftrunc(inst);
    case SI2SS:
    case SI2SD:
    case UI2SS:
    case UI2SD:
    case SS2SI:
    case SD2SI:
    case SS2UI:
    case SD2UI:
        select_cvt(inst);
        break;
    case Cmpeq:
    case Cmpne:
    case Cmpoeq:
    case Cmpone:
    case Cmpuneq:
    case Cmpunne:
    case Cmpslt:
    case Cmpsle:
    case Cmpsgt:
    case Cmpsge:
    case Cmpult:
    case Cmpule:
    case Cmpugt:
    case Cmpuge:
    case Cmpolt:
    case Cmpole:
    case Cmpogt:
    case Cmpoge:
    case Cmpunlt:
    case Cmpunle:
    case Cmpungt:
    case Cmpunge:
        select_cmp(inst);
        break;
    }
}

void InstSelection::select_const(Instruction* inst) {
    amd64::Opcode opcode;
    switch (inst->width()) {
    case WD_Byte:
        opcode = amd64::Opcode::MOV8;
        break;
    case WD_Short:
        opcode = amd64::Opcode::MOV16;
        break;
    case WD_Half:
        opcode = amd64::Opcode::MOV32;
        break;
    case WD_Word:
        opcode = amd64::Opcode::MOV64;
        break;
    case WD_Single:
        opcode = amd64::Opcode::MOVSS;
        break;
    case WD_Double:
        opcode = amd64::Opcode::MOVSD;
        break;
    default:
        assert(false);
    }

    MachineInst {
        opcode,
        { lower(inst->operands()[0]), lower(inst->operands()[1]) },
        m_insert
    };
}

void InstSelection::select_move(Instruction* inst) {

}

void InstSelection::select_lea(Instruction* inst) {

}

void InstSelection::select_copy(Instruction* inst) {

}

void InstSelection::select_jump(Instruction* inst) {

}

void InstSelection::select_branch(Instruction* inst) {

}

void InstSelection::select_set(Instruction* inst) {

}

void InstSelection::select_return(Instruction* inst) {
    if (inst->num_operands() > 0) {

    }

    MachineInst(amd64::Opcode::RET, {}, m_insert);
}

void InstSelection::select_call(Instruction* inst) {
    MachineInst(amd64::Opcode::CALL, { lower(inst->first()) }, m_insert);
}

void InstSelection::select_arith(Instruction* inst) {

}

void InstSelection::select_crement(Instruction* inst) {

}

void InstSelection::select_neg(Instruction* inst) {

}

void InstSelection::select_not(Instruction* inst) {

}

void InstSelection::select_logic(Instruction* inst) {

}

void InstSelection::select_shift(Instruction* inst) {

}

void InstSelection::select_sext(Instruction* inst) {

}

void InstSelection::select_zext(Instruction* inst) {

}

void InstSelection::select_fext(Instruction* inst) {

}

void InstSelection::select_trunc(Instruction* inst) {

}

void InstSelection::select_ftrunc(Instruction *inst) {

}

void InstSelection::select_cvt(Instruction* inst) {

}

void InstSelection::select_cmp(Instruction* inst) {

}
