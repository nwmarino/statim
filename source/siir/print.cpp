#include "siir/basicblock.hpp"
#include "siir/cfg.hpp"
#include "siir/constant.hpp"
#include "siir/function.hpp"
#include "siir/global.hpp"
#include "siir/instruction.hpp"
#include "siir/local.hpp"
#include "siir/type.hpp"
#include "siir/inlineasm.hpp"

#include <iomanip>
#include <iostream>

using namespace stm;
using namespace stm::siir;

static void print_global(std::ostream& os, Global* global) {
    os << global->get_name() << " :: ";

    switch (global->get_linkage()) {
    case Global::LINKAGE_INTERNAL:
        os << "$internal ";
        break;
    case Global::LINKAGE_EXTERNAL:
        os << "$external ";
        break;
    }

    if (global->is_read_only())
        os << "readonly ";

    os << global->get_type()->to_string();

    if (global->has_initializer()) {
        os << ' ';
        global->get_initializer()->print(os);
    }

    os << "\n";
}

static void print_inst(std::ostream& os, Instruction* inst) {
    if (inst->is_def())
        os << 'v' << inst->result_id() << " = ";

    os << opcode_to_string(inst->opcode()) << ' ';

    if (inst->is_def()) {
        os << inst->get_type()->to_string() << (inst->is_call() ? " " : ", ");
    }

    for (u32 idx = 0, e = inst->num_operands(); idx != e; ++idx) {
        Value* operand = inst->get_operand(idx);

        if (operand->has_type()) {
            if (!inst->has_type() && (!inst->is_call() || idx != 0)) {
                os << operand->get_type()->to_string() << ' ';
            } else if (inst->has_type() && !inst->is_call()
              && (*inst->get_type() != *operand->get_type())) {
                os << operand->get_type()->to_string() << ' ';
            }
        }

        operand->print(os);

        if (inst->is_call() && idx == 0) {
            os << '(';
        } else if (idx + 1 != e) {
            os << ", ";
        }
    }

    if (inst->is_call())
        os << ')';

    if (inst->is_load() || inst->is_store()) {
        os << ", align " << inst->get_data();
    }

    if (inst->is_def()) {
        os << " ... " << inst->num_uses() << " uses";
    }

    os << '\n';
}

static void print_local(std::ostream& os, Local* local) {
    os << '_' << local->get_name()  << ": " << 
        local->get_allocated_type()->to_string() << ", align " << 
        local->get_alignment() << " ... " << local->num_uses() << " uses\n";
}

static void print_block(std::ostream& os, BasicBlock* blk) {
    os << "    bb" << blk->get_number();
    
    os << ": {\n";

    if (blk->has_preds()) {
        os << "        ... preds: ";

        for (u32 idx = 0, e = blk->num_preds(); idx != e; ++idx) {
            os << "bb" << blk->preds()[idx]->get_number();
            if (idx + 1 != e)
                os << ", ";
        }

        if (!blk->has_succs())
            os << '\n';
    }

    if (blk->has_succs()) {
        if (blk->has_preds())
            os << ", succs: ";
        else
            os << "        ... succs: ";

        for (u32 idx = 0, e = blk->num_succs(); idx != e; ++idx) {
            os << "bb" << blk->succs()[idx]->get_number();
            if (idx + 1 != e)
                os << ", ";
        }

        os << '\n';
    }

    if (blk->has_preds() || blk->has_succs())
        os << '\n';

    for (auto curr = blk->front(); curr; curr = curr->next()) {
        os << "        ";
        print_inst(os, curr);
    }

    os << "    }\n";
}

static void print_arg(std::ostream& os, Argument* arg) {
    os << arg->get_name() << ": " << arg->get_type()->to_string(); 
}

static void print_function(std::ostream& os, Function* function) {
    os << function->get_name() << " :: (";
    
    for (u32 idx = 0, e = function->num_args(); idx != e; ++idx) {
        print_arg(os, function->get_arg(idx));
        if (idx + 1 != e) 
            os << ", ";
    }

    os << ") -> ";
    
    if (function->get_return_type())
        os << function->get_return_type()->to_string();
    else
        os << "void";

    if (function->empty()) {
        os << "\n";
        return;
    } else {
        os << " {\n";
    }

    if (!function->locals().empty()) {
        for (auto [ name, local ] : function->locals()) {
            os << "    ";
            print_local(os, local);
        }

        os << '\n';
    }

    for (auto curr = function->front(); curr; curr = curr->next()) {
        print_block(os, curr);
        if (curr->next())
            os << '\n';
    }

    os << "}\n";
}

void CFG::print(std::ostream& os) const {
    if (!m_types_structs.empty()) {
        for (auto [ name, type ] : m_types_structs) {
            os << name << " :: {\n";

            for (u32 idx = 0, e = type->fields().size(); idx != e; ++idx) {
                os << "    " << type->get_field(idx)->to_string();
                if (idx + 1 != e)
                    os << ','; 
                
                os << '\n';
            }
            
            os << "}\n\n";
        }
    }

    if (!m_globals.empty()) {
        for (auto& [ name, global ] : m_globals)
            print_global(os, global);

        os << '\n';
    }

    if (!m_functions.empty()) {
        u32 idx = 0, e = m_functions.size();
        for (auto& [ name, function ] : m_functions) {
            print_function(os, function);
            if (++idx != e)
                os << '\n';
        }
    }

    os << '\n';
}

void Global::print(std::ostream& os) const {
    os << m_name;
}

void Function::print(std::ostream& os) const {
    os << m_name;
}

void Argument::print(std::ostream& os) const {
    os << m_name;
}

void Local::print(std::ostream& os) const {
    os << '_' << m_name;
}

void BasicBlock::print(std::ostream& os) const {
    os << "bb" << get_number();
}

void ConstantInt::print(std::ostream& os) const {
    os << m_value;
}

void ConstantFP::print(std::ostream& os) const {
    os << std::setprecision(6) << m_value;
}

void ConstantNull::print(std::ostream& os) const {
    os << "null";
}

void BlockAddress::print(std::ostream& os) const {
    os << "bb" << m_block->get_number();
}

void ConstantString::print(std::ostream& os) const {
    os << '"';

    for (u32 idx = 0, e = m_value.size(); idx != e; ++idx) {
        switch (m_value[idx]) {
        case '\\':
            os << "\\\\";
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
            os << m_value[idx];
            break;
        }
    }

    os << '"';
}

void PhiOperand::print(std::ostream& os) const {
    m_pred->print(os);
    os << ' ';
    m_value->print(os);
}

void Instruction::print(std::ostream& os) const {
    assert(is_def());
    os << 'v' << m_result;
}

void InlineAsm::print(std::ostream& os) const {
    os << "asm \"";
    
    for (u32 idx = 0, e = m_iasm.size(); idx != e; ++idx) {
        switch (m_iasm[idx]) {
        case '\\':
            os << "\\\\";
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
            os << m_iasm[idx];
            break;
        }
    }

    if (constraints().size() == 0) {
        os << "\" ";
        return;
    }

    os << "\" : ";

    for (u32 idx = 0, e = constraints().size(); idx != e; ++idx) {
        os << '"' << constraints().at(idx) << '"';
        if (idx + 1 != e)
            os << ", ";
    }

    os << ' ';
}
