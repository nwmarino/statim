#include "siir/argument.hpp"
#include "siir/basicblock.hpp"
#include "siir/cfg.hpp"
#include "siir/constant.hpp"
#include "siir/function.hpp"
#include "siir/global.hpp"
#include "siir/instruction.hpp"
#include "siir/local.hpp"
#include "siir/type.hpp"

#include <iomanip>
#include <unordered_map>

using namespace stm;
using namespace stm::siir;

static std::unordered_map<const void*, u32> s_values = {};

static void print_local(std::ostream& os, Local* local);
static void print_block(std::ostream& os, BasicBlock* blk);
static void print_inst(std::ostream& os, Instruction* inst);

static std::string name(const Value* value) {
    assert(value->get_type() != nullptr && "cannot get name for untyped value");
    
    if (value->has_name())
        return value->get_name();

    auto it = s_values.find(value);
    if (it != s_values.end())
        return std::to_string(it->second);

    u32 v = s_values.size();
    s_values.emplace(value, v);
    return std::to_string(v);
}

static void print_global(std::ostream& os, Global* global) {
    os << name(global) << " :: ";

    switch (global->get_linkage()) {
    case Global::Internal:
        os << "$internal ";
        break;
    case Global::External:
        os << "$external ";
        break;
    }

    if (global->is_read_only())
        os << "ro ";

    os << global->get_type()->to_string();

    if (global->has_initializer()) {
        os << ' ';
        global->get_initializer()->print(os);
    }

    os << "\n";
}

static void print_function(std::ostream& os, Function* function) {
    assert(function);

    os << name(function) << " :: (";
    
    for (u32 idx = 0, e = function->num_args(); idx != e; ++idx) {
        function->get_arg(idx)->print(os);
        if (idx + 1 != e) os << ", ";
    }

    os << ") -> ";
    
    if (function->get_return_type())
        os << function->get_return_type()->to_string();
    else
        os << "void";

    os << " {\n";

    if (!function->get_locals().empty()) {
        for (auto [ name, local ] : function->get_locals()) {
            assert(local);
            os << "    ";
            print_local(os, local);
        }

        os << '\n';
    }

    for (auto curr = function->front(); curr; curr = curr->next()) {
        assert(curr);
        print_block(os, curr);

        if (curr->next())
            os << '\n';
    }

    os << "}\n";
}

static void print_farg(std::ostream& os, FunctionArgument* arg) {
    os << name(arg) << ": " << arg->get_type()->to_string(); 
}

static void print_local(std::ostream& os, Local* local) {
    os << name(local)  << ": " << local->get_allocated_type()->to_string() << 
        ", align " << local->get_alignment() << "\n";
}

static void print_block(std::ostream& os, BasicBlock* blk) {
    os << "    bb" << blk->get_number() << ": {\n";

    for (auto curr = blk->front(); curr; curr = curr->next()) {
        os << "        ";
        print_inst(os, curr);
    }

    os << "    }\n";
}

static void print_barg(std::ostream& os, BlockArgument* arg) {
    os << name(arg) << ": " << arg->get_type()->to_string(); 
}

static void print_inst(std::ostream& os, ConstInst* inst) {
    os << '%' << name(inst) << " := const " << inst->get_type()->to_string() << 
        ' ';
    inst->get_constant()->print(os);
    os << '\n';
}

static void print_inst(std::ostream& os, StoreInst* inst) {
    os << "str ";
    inst->get_value()->print(os);
    os << ", ";
    inst->get_destination()->print(os);
    os << ", align " << inst->get_alignment() << '\n';
}

static void print_inst(std::ostream& os, LoadInst* inst) {
    os << '%' << name(inst) << " := ld " << inst->get_type()->to_string() <<
        ", ";
    inst->get_source()->print(os);
    os << ", align " << inst->get_alignment() << '\n';
}

static void print_inst(std::ostream& os, APInst* inst) {
    os << '%' << name(inst) << " := ap";

    if (inst->is_deref())
        os << 'd';

    os << ' ' << inst->get_type()->to_string() << ", ";
    inst->get_source()->print(os);
    os << ", ";
    inst->get_index()->print(os);
    os << '\n';
}

static void print_inst(std::ostream& os, SelectInst* inst) {
    os << '%' << name(inst) << " := sel ";
    inst->get_condition()->print(os);
    os << ", ";
    inst->get_true_value()->print(os);
    os << ", ";
    inst->get_false_value()->print(os);
    os << '\n';
}

static void print_inst(std::ostream& os, BrifInst* inst) {
    os << "br_if ";
    inst->get_condition()->print(os);
    os << ", ";
    inst->get_true_dest()->print(os);
    os << ", ";
    inst->get_false_dest()->print(os);
    os << '\n';
}

static void print_inst(std::ostream& os, JmpInst* inst) {
    os << "jmp ";
    inst->get_destination()->print(os);
    os << '\n';
}

static void print_inst(std::ostream& os, RetInst* inst) {
    os << "ret";

    if (inst->get_return_value()) {
        os << ' ';
        inst->get_return_value()->print(os);
    }

    os << '\n';
}

static void print_inst(std::ostream& os, AbortInst* inst) {
    os << "abort";
}

static void print_inst(std::ostream& os, UnreachableInst* inst) {
    os << "unreachable";
}

static void print_inst(std::ostream& os, CallInst* inst) {
    if (inst->get_type())
        os << '%' << name(inst) << " := ";

    os << "call ";

    inst->get_callee()->print(os);
    os << '(';

    for (u32 idx = 0, e = inst->num_args(); idx != e; ++idx) {
        inst->get_arg(idx)->print(os);
        if (idx + 1 != e)
            os << ", ";
    }

    os << ")\n";
}

static void print_inst(std::ostream& os, CmpInst* inst) {
    os << '%' << name(inst) << " := cmp_";

    switch (inst->get_predicate()) {
    case CmpInst::CMP_Eq:
        os << "eq";
        break;
    case CmpInst::CMP_Ne:
        os << "ne";
        break;
    case CmpInst::CMP_Oeq:
        os << "oeq";
        break;
    case CmpInst::CMP_One:
        os << "one";
        break;
    case CmpInst::CMP_Uneq:
        os << "uneq";
        break;
    case CmpInst::CMP_Unne:
        os << "unne";
        break;
    case CmpInst::CMP_Slt:
        os << "slt";
        break;
    case CmpInst::CMP_Sle:
        os << "sle";
        break;
    case CmpInst::CMP_Sgt:
        os << "sgt";
        break;
    case CmpInst::CMP_Sge:
        os << "sge";
        break;
    case CmpInst::CMP_Ult:
        os << "ult";
        break;
    case CmpInst::CMP_Ule:
        os << "ule";
        break;
    case CmpInst::CMP_Ugt:
        os << "ugt";
        break;
    case CmpInst::CMP_Uge:
        os << "uge";
        break;
    case CmpInst::CMP_Olt:
        os << "olt";
        break;
    case CmpInst::CMP_Ole:
        os << "ole";
        break;
    case CmpInst::CMP_Ogt:
        os << "ogt";
        break;
    case CmpInst::CMP_Oge:
        os << "oge";
        break;
    case CmpInst::CMP_Unlt:
        os << "unlt";
        break;
    case CmpInst::CMP_Unle:
        os << "unle";
        break;
    case CmpInst::CMP_Ungt:
        os << "ungt";
        break;
    case CmpInst::CMP_Unge:
        os << "unge";
        break;
    }

    os << ' ';
    inst->get_lhs_value()->print(os);
    os << ", ";
    inst->get_rhs_value()->print(os);
    os << '\n';
}

static void print_inst(std::ostream& os, BinopInst* inst) {
    os << '%' << name(inst) << " := ";
    
    switch (inst->get_operator()) {
    case BinopInst::BINOP_Add:
        os << "add";
        break;
    case BinopInst::BINOP_FAdd:
        os << "fadd";
        break;
    case BinopInst::BINOP_Sub:
        os << "sub";
        break;
    case BinopInst::BINOP_FSub:
        os << "fsub";
        break;
    case BinopInst::BINOP_Mul:
        os << "mul";
        break;
    case BinopInst::BINOP_FMul:
        os << "fmul";
        break;
    case BinopInst::BINOP_SDiv:
        os << "sdiv";
        break;
    case BinopInst::BINOP_UDiv:
        os << "udiv";
        break;
    case BinopInst::BINOP_FDiv:
        os << "fdiv";
        break;
    case BinopInst::BINOP_SRem:
        os << "srem";
        break;
    case BinopInst::BINOP_URem:
        os << "urem";
        break;
    case BinopInst::BINOP_FRem:
        os << "frem";
        break;
    case BinopInst::BINOP_And:
        os << "and";
        break;
    case BinopInst::BINOP_Or:
        os << "or";
        break;
    case BinopInst::BINOP_Xor:
        os << "xor";
        break;
    case BinopInst::BINOP_Shl:
        os << "shl";
        break;
    case BinopInst::BINOP_Shr:
        os << "shr";
        break;
    case BinopInst::BINOP_Sar:
        os << "sar";
        break;
    }

    os << ' ';
    inst->get_lhs_value()->print(os);
    os << ", ";
    inst->get_rhs_value()->print(os);
    os << '\n';
}

static void print_inst(std::ostream& os, UnopInst* inst) {
    os << '%' << name(inst) << " := ";
    
    switch (inst->get_operator()) {
    case UnopInst::UNOP_Not:
        os << "not";
        break;
    case UnopInst::UNOP_Neg:
        os << "neg";
        break;
    case UnopInst::UNOP_FNeg:
        os << "fneg";
        break;
    case UnopInst::UNOP_SExt:
        os << "sext";
        break;
    case UnopInst::UNOP_ZExt:
        os << "zext";
        break;
    case UnopInst::UNOP_FExt:
        os << "fext";
        break;
    case UnopInst::UNOP_Trunc:
        os << "trunc";
        break;
    case UnopInst::UNOP_FTrunc:
        os << "ftrunc";
        break;
    case UnopInst::UNOP_SI2FP:
        os << "si2fp";
        break;
    case UnopInst::UNOP_UI2FP:
        os << "ui2fp";
        break;
    case UnopInst::UNOP_FP2SI:
        os << "fp2si";
        break;
    case UnopInst::UNOP_FP2UI:
        os << "fp2ui";
        break;
    case UnopInst::UNOP_P2I:
        os << "p2i";
        break;
    case UnopInst::UNOP_I2P:
        os << "i2p";
        break;
    case UnopInst::UNOP_Reint:
        os << "reint";
        break;
    }

    os << ' ';
    inst->get_value()->print(os);
    os << '\n';
}

static void print_inst(std::ostream& os, Instruction* inst) {
    switch (inst->get_op()) {
    case Instruction::Const:
        return print_inst(os, static_cast<ConstInst*>(inst));
    case Instruction::Store:
        return print_inst(os, static_cast<StoreInst*>(inst));
    case Instruction::Load:
        return print_inst(os, static_cast<LoadInst*>(inst));
    case Instruction::AP:
        return print_inst(os, static_cast<APInst*>(inst));
    case Instruction::Select:
        return print_inst(os, static_cast<SelectInst*>(inst));
    case Instruction::Brif:
        return print_inst(os, static_cast<BrifInst*>(inst));
    case Instruction::Jmp:
        return print_inst(os, static_cast<JmpInst*>(inst));
    case Instruction::Ret:
        return print_inst(os, static_cast<RetInst*>(inst));
    case Instruction::Abort:
        return print_inst(os, static_cast<AbortInst*>(inst));
    case Instruction::Unreachable:
        return print_inst(os, static_cast<UnreachableInst*>(inst));
    case Instruction::Call:
        return print_inst(os, static_cast<CallInst*>(inst));
    case Instruction::Cmp:
        return print_inst(os, static_cast<CmpInst*>(inst));
    case Instruction::Binop:
        return print_inst(os, static_cast<BinopInst*>(inst));
    case Instruction::Unop:
        return print_inst(os, static_cast<UnopInst*>(inst));
    }
}

void CFG::print(std::ostream& os) const {
    s_values.clear();

    if (!m_types_structs.empty()) {
        for (auto [ name, type ] : m_types_structs) {
            type->print(os);
            os << '\n';
        }

        os << '\n';
    }

    if (!m_globals.empty()) {
        for (auto& [ name, global ] : m_globals) {
            print_global(os, global);
        }

        os << '\n';
    }

    if (!m_functions.empty()) {
        for (auto& [ name, function ] : m_functions) {
            print_function(os, function);
            os << '\n';
        }

        os << '\n';
    }
}

void StructType::print(std::ostream& os) const {
    os << m_name << " :: {\n";

    for (u32 idx = 0, e = num_fields(); idx != e; ++idx) {
        os << "    " << m_fields[idx]->to_string();
        if (idx + 1 != e)
            os << ','; 
        
        os << '\n';
    }
    
    os << "}\n";
}

void Global::print(std::ostream& os) const {
    os << m_type->to_string() << " %" << name(this);
}

void Function::print(std::ostream& os) const {
    os << get_return_type()->to_string() << " @" << m_name;
}

void FunctionArgument::print(std::ostream& os) const {
    os << m_type->to_string() << " %" << name(this);
}

void Local::print(std::ostream& os) const {
    os << m_type->to_string() << " %" << name(this); 
}

void BasicBlock::print(std::ostream& os) const {
    os << "bb" << get_number();
}

void BlockArgument::print(std::ostream& os) const {
    os << m_type->to_string() << " %" << name(this);
}

void ConstantInt::print(std::ostream& os) const {
    os << m_type->to_string() << ' ' << m_value;
}

void ConstantFP::print(std::ostream& os) const {
    os << m_type->to_string() << ' ' << std::setprecision(6) << m_value;
}

void ConstantNull::print(std::ostream& os) const {
    os << m_type->to_string() << " null";
}

void BlockAddress::print(std::ostream& os) const {
    os << "bb" << m_block->get_number();
}

void ConstInst::print(std::ostream& os) const {
    os << m_type->to_string() << " %" << name(this);
}

void StoreInst::print(std::ostream& os) const {
    assert(false && "cannot print store instruction by reference");
}

void LoadInst::print(std::ostream& os) const {
    os << m_type->to_string() << " %" << name(this);
}

void APInst::print(std::ostream& os) const {
    os << m_type->to_string() << " %" << name(this);
}

void SelectInst::print(std::ostream& os) const {
    os << m_type->to_string() << " %" << name(this);
}

void BrifInst::print(std::ostream& os) const {
    assert(false && "cannot print brif instruction by reference");
}

void JmpInst::print(std::ostream& os) const {
    assert(false && "cannot print jump instruction by reference");
}

void RetInst::print(std::ostream& os) const {
    assert(false && "cannot print return instruction by reference");
}

void AbortInst::print(std::ostream& os) const {
    assert(false && "cannot print abort instruction by reference");

}

void UnreachableInst::print(std::ostream& os) const {
    assert(false && "cannot print unreachable instruction by reference");   
}

void CallInst::print(std::ostream& os) const {
    assert(m_type);

    os << m_type->to_string() << " %" << name(this);
}

void CmpInst::print(std::ostream& os) const {
    os << m_type->to_string() << " %" << name(this);
}

void BinopInst::print(std::ostream& os) const {
    os << m_type->to_string() << " %" << name(this);
}

void UnopInst::print(std::ostream& os) const {
    os << m_type->to_string() << " %" << name(this);
}
