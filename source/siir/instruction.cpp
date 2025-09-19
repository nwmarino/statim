#include "siir/basicblock.hpp"
#include "siir/cfg.hpp"
#include "siir/user.hpp"
#include <string>
#include "siir/instruction.hpp"

using namespace stm;
using namespace stm::siir;

PhiOperand::PhiOperand(Value* value, BasicBlock* pred) 
    : Value(value->get_type()), m_value(value), m_pred(pred) {}

std::string stm::siir::opcode_to_string(Opcode op) {
    switch (op) {
    case INST_OP_NOP:
        return "Nop";
    case INST_OP_CONSTANT:
        return "Constant";
    case INST_OP_STRING:
        return "String";
    case INST_OP_LOAD:
        return "Load";
    case INST_OP_STORE:
        return "Store";
    case INST_OP_ACCESS_PTR:
        return "AP";
    case INST_OP_SELECT:
        return "Select";
    case INST_OP_BRANCH_IF:
        return "BranchIf";
    case INST_OP_JUMP:
        return "Jump";
    case INST_OP_PHI:
        return "Phi";
    case INST_OP_RETURN:
        return "Return";
    case INST_OP_ABORT:
        return "Abort";
    case INST_OP_UNREACHABLE:
        return "Unreachable";
    case INST_OP_CALL:
        return "Call";
    case INST_OP_IADD:
        return "IAdd";
    case INST_OP_FADD:
        return "FAdd";
    case INST_OP_ISUB:
        return "ISub";
    case INST_OP_FSUB:
        return "FSub";
    case INST_OP_SMUL:
        return "SMul";
    case INST_OP_UMUL:
        return "UMul";
    case INST_OP_FMUL:
        return "FMul";
    case INST_OP_SDIV:
        return "SDiv";
    case INST_OP_UDIV:
        return "UDiv";
    case INST_OP_FDIV:
        return "FDiv";
    case INST_OP_SREM:
        return "SRem";
    case INST_OP_UREM:
        return "URem";
    case INST_OP_AND:
        return "And";
    case INST_OP_OR:
        return "Or";
    case INST_OP_XOR:
        return "Xor";
    case INST_OP_SHL:
        return "Shl";
    case INST_OP_SHR:
        return "Shr";
    case INST_OP_SAR:
        return "Sar";
    case INST_OP_NOT:
        return "Not";
    case INST_OP_INEG:
        return "INeg";
    case INST_OP_FNEG:
        return "FNeg";
    case INST_OP_SEXT:
        return "SExt";
    case INST_OP_ZEXT:
        return "ZExt";
    case INST_OP_FEXT:
        return "FExt";
    case INST_OP_ITRUNC:
        return "ITrunc";
    case INST_OP_FTRUNC:
        return "FTrunc";
    case INST_OP_SI2FP:
        return "SI2FP";
    case INST_OP_UI2FP:
        return "UI2FP";
    case INST_OP_FP2SI:
        return "FP2SI";
    case INST_OP_FP2UI:
        return "FP2UI";
    case INST_OP_P2I:
        return "P2I";
    case INST_OP_I2P:
        return "I2P";
    case INST_OP_REINTERPET:
        return "Reinterpret";
    case INST_OP_CMP_IEQ:
        return "CmpIEQ";
    case INST_OP_CMP_INE:
        return "CmpINE";
    case INST_OP_CMP_OEQ:
        return "CmpOEQ";
    case INST_OP_CMP_ONE:
        return "CmpONE";
    case INST_OP_CMP_UNEQ:
        return "CmpUNEQ";
    case INST_OP_CMP_UNNE:
        return "CmpUNNE";
    case INST_OP_CMP_SLT:
        return "CmpSLT";
    case INST_OP_CMP_SLE:
        return "CmpSLE";
    case INST_OP_CMP_SGT:
        return "CmpSGT";
    case INST_OP_CMP_SGE:
        return "CmpSGE";
    case INST_OP_CMP_ULT:
        return "CmpULT";
    case INST_OP_CMP_ULE:
        return "CmpULE";
    case INST_OP_CMP_UGT:
        return "CmpUGT";
    case INST_OP_CMP_UGE:
        return "CmpUGE";
    case INST_OP_CMP_OLT:
        return "CmpOLT";
    case INST_OP_CMP_OLE:
        return "CmpOLE";
    case INST_OP_CMP_OGT:
        return "CmpOGT";
    case INST_OP_CMP_OGE:
        return "CmpOGE";
    case INST_OP_CMP_UNLT:
        return "CmpUNLT";
    case INST_OP_CMP_UNLE:
        return "CmpUNLE";
    case INST_OP_CMP_UNGT:
        return "CmpUNGT";
    case INST_OP_CMP_UNGE:
        return "CmpUNGE";
    default:
        return "Unknown";
    }
}

Instruction::Instruction(Opcode opcode, BasicBlock* parent,
                         const std::vector<Value*>& operands)
    : User(operands, nullptr), m_result(0), m_opcode(opcode), 
      m_parent(parent) {}

Instruction::Instruction(u32 result, const Type* type, Opcode opcode, 
                         BasicBlock* parent, 
                         const std::vector<Value*>& operands)
    : User(operands, type), m_result(result), m_opcode(opcode), 
      m_parent(parent) {}

const Value* Instruction::get_operand(u32 i) const {
    assert(i <= num_operands());
    return m_operands[i]->get_value();
}

void Instruction::prepend_to(BasicBlock* blk) {
    assert(blk && "blk cannot be null");
    blk->push_front(this);
}

void Instruction::append_to(BasicBlock* blk) {
    assert(blk && "blk cannot be null");
    blk->push_back(this);
}

void Instruction::insert_before(Instruction* inst) {
    assert(inst && "inst cannot be null");

    if (inst->prev())
        inst->prev()->set_next(this);

    m_prev = inst->prev();
    m_next = inst;
    inst->set_prev(this);
    set_parent(inst->get_parent());
}

void Instruction::insert_after(Instruction* inst) {
    assert(inst && "inst cannot be null");

    if (inst->next())
        inst->next()->set_prev(this);

    m_prev = inst;
    m_next = inst->next();
    inst->set_next(this);
    set_parent(inst->get_parent());
}

void Instruction::detach_from_parent() {
    assert(m_parent && "cannot detach a free-floating instruction");

    if (m_prev) {
        m_prev->m_next = m_next;
    } else {
        // No instruction before this one, so it must be the first instruction
        // in the parent block; have to update the parent block links.
        if (m_next) {
            m_parent->set_front(m_next);
        } else {
            // This was the only instruction in the block.
            m_parent->set_front(nullptr);
            m_parent->set_back(nullptr);
        }
    }

    if (m_next) {
        m_next->m_prev = m_prev;
    } else {
        // No instruction after this one, so it must be the last instruction in
        // the parent block.
        if (m_prev) {
            m_parent->set_back(m_prev);     
        } else {
            // This was the only instruction in the block.
            m_parent->set_front(nullptr);
            m_parent->set_back(nullptr);
        }
    }

    m_parent = nullptr;
    m_prev = m_next = nullptr;
}

bool Instruction::is_terminator() const {
    switch (opcode()) {
    case INST_OP_BRANCH_IF:
    case INST_OP_JUMP:
    case INST_OP_RETURN:
    case INST_OP_ABORT:
    case INST_OP_UNREACHABLE:
        return true;
    default:
        return false;
    }
}

bool Instruction::is_comparison() const {
    switch (opcode()) {
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
        return true;
    default:
        return false;
    }
}

bool Instruction::is_ordered_cmp() const {
    switch (opcode()) {
    case INST_OP_CMP_OEQ:
    case INST_OP_CMP_ONE:
    case INST_OP_CMP_OLT:
    case INST_OP_CMP_OLE:
    case INST_OP_CMP_OGT:
    case INST_OP_CMP_OGE:
        return true;
    default:
        return false;
    }
}

bool Instruction::is_unordered_cmp() const {
    switch (opcode()) {
    case INST_OP_CMP_UNEQ:
    case INST_OP_CMP_UNNE:
    case INST_OP_CMP_UNLT:
    case INST_OP_CMP_UNLE:
    case INST_OP_CMP_UNGT:
    case INST_OP_CMP_UNGE:
        return true;
    default:
        return false;
    }
}

bool Instruction::is_cast() const {
    switch (opcode()) {
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
        return true;
    default:
        return false;
    }
}

bool Instruction::operates_on_floats() const {
    switch (opcode()) {
    case INST_OP_CMP_OEQ:
    case INST_OP_CMP_ONE:
    case INST_OP_CMP_UNEQ:
    case INST_OP_CMP_UNNE:
    case INST_OP_CMP_OLT:
    case INST_OP_CMP_OLE:
    case INST_OP_CMP_OGT:
    case INST_OP_CMP_OGE:
    case INST_OP_CMP_UNLT:
    case INST_OP_CMP_UNLE:
    case INST_OP_CMP_UNGT:
    case INST_OP_CMP_UNGE:
    case INST_OP_FADD:
    case INST_OP_FSUB:
    case INST_OP_FMUL:
    case INST_OP_FDIV:
    case INST_OP_FNEG:
    case INST_OP_FEXT:
    case INST_OP_FTRUNC:
    case INST_OP_FP2SI:
    case INST_OP_FP2UI:
        return true;
    default:
        return false;
    }
}

void Instruction::add_incoming(CFG& cfg, Value* value, BasicBlock* pred) {
    PhiOperand* incoming = new PhiOperand(value, pred);
    cfg.m_pool_incomings.push_back(incoming);
    User::add_operand(incoming);
}

bool Instruction::is_trivially_dead() const {
    if (result_id() == 0 || Value::used())
        return false;

    switch (opcode()) {
    case INST_OP_NOP:
    case INST_OP_CONSTANT:
    case INST_OP_STRING:
    case INST_OP_LOAD:
    case INST_OP_ACCESS_PTR:
    case INST_OP_SELECT:
    case INST_OP_PHI:
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
        return true;
    default:
        return false;
    }
}
