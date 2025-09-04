#include "siir/basicblock.hpp"
#include "siir/cfg.hpp"
#include "siir/constant.hpp"
#include "siir/function.hpp"
#include "siir/global.hpp"
#include "siir/instruction.hpp"
#include "siir/local.hpp"
#include "siir/type.hpp"

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

    if (inst->is_def())
        os << inst->get_type()->to_string() << (inst->is_call() ? " " : ", ");

    for (u32 idx = 0, e = inst->num_operands(); idx != e; ++idx) {
        Value* operand = inst->get_operand(idx);

        if (operand->has_type()) {
            if (!inst->has_type()) {
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
        } else if (inst->is_call()) {
            os << ')';
        }
    }

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
    
    if (blk->has_preds()) {
        os << '(';

        for (u32 idx = 0, e = blk->num_preds(); idx != e; ++idx) {
            os << "bb" << blk->preds()[idx]->get_number();
            if (idx + 1 != e)
                os << ", ";
        }

        os << ')';
    }
    
    os << ": {\n";

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

    os << " {\n";

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
        for (auto& [ name, function ] : m_functions)
            print_function(os, function);
    }
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

void PhiOperand::print(std::ostream& os) const {
    m_pred->print(os);
    os << ' ';
    m_value->print(os);
}

void Instruction::print(std::ostream& os) const {
    assert(is_def());
    os << 'v' << m_result;
}
