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

using namespace stm;
using namespace stm::siir;

static u32 s_value = 0;

void CFG::print(std::ostream& os) const {
    if (!m_types_structs.empty()) {
        for (auto [ name, type ] : m_types_structs) {
            type->print(os);
            os << '\n';
        }

        os << '\n';
    }

    if (!m_globals.empty()) {
        for (auto [ name, global ] : m_globals) {
            global->print(os);
            os << '\n';
        }

        os << '\n';
    }

    if (!m_functions.empty()) {
        for (auto [ name, function ] : m_functions) {
            function->print(os);
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
    os << (has_name() ? std::to_string(s_value) : m_name) << " :: "; 

    switch (m_linkage) {
    case Internal:
        os << "$internal ";
        break;
    case External:
        os << "$external ";
        break;
    }

    if (m_read_only)
        os << "ro ";

    os << m_type->to_string();

    if (m_init) {
        os << ' ';
        m_init->print(os);
    }

    os << "\n";
}

void Function::print(std::ostream& os) const {
    os << (has_name() ? std::to_string(s_value) : m_name) << " :: (";
    
    for (u32 idx = 0, e = num_args(); idx != e; ++idx) {
        m_args[idx]->print(os);
        if (idx + 1 != e) os << ", ";
    }

    os << ") " << get_return_type()->to_string() << " {\n";

    if (!m_locals.empty()) {
        for (auto [ name, local ] : m_locals) {
            os << "    ";
            local->print(os);
        }

        os << '\n';
    }

    for (auto curr = m_front; curr; curr = curr->next()) {
        curr->print(os);

        if (curr->next())
            os << '\n';
    }

    os << "}\n";
}

void FunctionArgument::print(std::ostream& os) const {
    os << (has_name() ? std::to_string(s_value) : m_name) << ": " << 
        m_type->to_string();
}

void Local::print(std::ostream& os) const {
    os << (has_name() ? std::to_string(s_value) : m_name) << ": " <<
        m_alloc_type->to_string() << ", align " << m_align << "\n";
}

void BasicBlock::print(std::ostream& os) const {
    os << (has_name() ? "bb" + std::to_string(s_value) : m_name) << ": {\n";

    for (auto curr = m_front; curr; curr = curr->next()) {
        os << "    ";
        curr->print(os);

        if (curr->next())
            os << '\n';
    }

    os << "}\n";
}

void BlockArgument::print(std::ostream& os) const {
    os << (has_name() ? std::to_string(s_value) : m_name) << ": " << 
        m_type->to_string();
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
    os << m_block->has_name();
}

void ConstInst::print(std::ostream& os) const {
    
}

void StoreInst::print(std::ostream& os) const {
    
}

void LoadInst::print(std::ostream& os) const {
    
}

void SelectInst::print(std::ostream& os) const {
    
}

void BrifInst::print(std::ostream& os) const {
    
}

void JmpInst::print(std::ostream& os) const {
    
}

void RetInst::print(std::ostream& os) const {
    
}

void AbortInst::print(std::ostream& os) const {
    
}

void UnreachableInst::print(std::ostream& os) const {
    
}

void CallInst::print(std::ostream& os) const {
    
}

void CmpInst::print(std::ostream& os) const {
    
}

void BinopInst::print(std::ostream& os) const {
    
}

void UnopInst::print(std::ostream& os) const {
    
}
