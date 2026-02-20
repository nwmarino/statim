//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Value.hpp"
#include "lir/graph/Instruction.hpp"

#include <format>

using namespace lir;

//>==---------------------------------------------------------------------------
//                          Instruction Implementation
//>==---------------------------------------------------------------------------

Instruction::Instruction(Type *type, BasicBlock *parent, uint32_t def, const Operands &ops)
  : User(type, ops), m_parent(parent), m_def(def) {}

void Instruction::detach() {
    assert(m_parent && "instruction does not belong to a basic block!");

    m_parent->remove(this);
}

void Instruction::prepend_to(BasicBlock *block) {
    assert(block && "block cannot be null!");

    block->prepend(this);
}

void Instruction::append_to(BasicBlock *block) {
    assert(block && "block cannot be null!");

    block->append(this);
}

void Instruction::insert_before(Instruction *inst) {
    assert(inst && "inst cannot be null!");

    if (inst->get_prev())
        inst->get_prev()->set_next(this);

    m_prev = inst->get_prev();
    m_next = inst;

    inst->set_prev(this);
    m_parent = inst->get_parent();
}

void Instruction::insert_after(Instruction *inst) {
    assert(inst && "inst cannot be null!");

    if (inst->get_next())
        inst->get_next()->set_prev(this);

    m_prev = inst;
    m_next = inst->get_next();

    inst->set_next(this);
    m_parent = inst->get_parent();
}

bool Instruction::is_trivially_dead() const {
    return false; // @Todo: Implement formally.

    if (!is_def() || Value::used())
        return false;

    return !dynamic_cast<const Call*>(this);
}

//>==---------------------------------------------------------------------------
//                          Const Implementation
//>==---------------------------------------------------------------------------

void Const::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << std::format("%{}: {}", m_def, m_type->to_string());
    } else if (policy == PrintPolicy::Def) {
        os << std::format("%{} := const <{}> ", m_def, get_type()->to_string());
        get_value()->print(os, PrintPolicy::Use);
        os << '\n';
    }
}

//>==---------------------------------------------------------------------------
//                          Load Implementation
//>==---------------------------------------------------------------------------

void Load::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << std::format("%{}: {}", m_def, m_type->to_string());
    } else if (policy == PrintPolicy::Def) {
        os << std::format("%{} := load <{}> ", m_def, get_type()->to_string());
        get_addr()->print(os, PrintPolicy::Use);
        os << std::format(" [{}]\n", m_align);
    }
}

//>==---------------------------------------------------------------------------
//                          Store Implementation
//>==---------------------------------------------------------------------------

void Store::print(std::ostream &os, PrintPolicy policy) const {
    assert(policy != PrintPolicy::Use && "store does not define a value!");

    if (policy == PrintPolicy::Def) {
        os << "store ";
        get_value()->print(os, PrintPolicy::Use);
        os << ", ";
        get_addr()->print(os, PrintPolicy::Use);
        os << std::format(" [{}]\n", get_align());
    }
}

//>==---------------------------------------------------------------------------
//                          Access Implementation
//>==---------------------------------------------------------------------------

void Access::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << std::format("%{}: {}", m_def, m_type->to_string());
    } else if (policy == PrintPolicy::Def) {
        os << std::format("%{} := access <{}> ", m_def, get_type()->to_string());
        get_base()->print(os, PrintPolicy::Use);
        os << ", ";
        get_index()->print(os, PrintPolicy::Use);
        os << '\n';
    }
}

//>==---------------------------------------------------------------------------
//                          Offptr Implementation
//>==---------------------------------------------------------------------------

void Offptr::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << std::format("%{}: {}", m_def, m_type->to_string());
    } else if (policy == PrintPolicy::Def) {
        os << std::format("%{} := offptr <{}> ", m_def, get_type()->to_string());
        get_base()->print(os, PrintPolicy::Use);
        os << ", ";
        get_index()->print(os, PrintPolicy::Use);
        os << '\n';
    }
}

//>==---------------------------------------------------------------------------
//                          Call Implementation
//>==---------------------------------------------------------------------------

void Call::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        assert(is_def() && "call does not define a value!");

        os << std::format("%{}: {}", m_def, m_type->to_string());
    } else if (policy == PrintPolicy::Def) {
        if (is_def())
            os << std::format("%{} := ", m_def);

        os << std::format("call <{}> ", get_type()->to_string());
        get_callee()->print(os, PrintPolicy::Use);
        os << " (";

        for (uint32_t i = 0, e = num_args(); i < e; ++i) {
            get_arg(i)->print(os, PrintPolicy::Use);
            if (i + 1 != e)
                os << ", ";
        }

        os << ")\n";
    }
}

//>==---------------------------------------------------------------------------
//                          Ret Implementation
//>==---------------------------------------------------------------------------

void Ret::print(std::ostream &os, PrintPolicy policy) const {
    assert(policy != PrintPolicy::Use && "ret does not define a value!");

    if (policy == PrintPolicy::Def) {
        os << "ret";

        if (has_value()) {
            os << ' ';
            get_value()->print(os, PrintPolicy::Use);
        }

        os << '\n';
    }
}

//>==---------------------------------------------------------------------------
//                          Jump Implementation
//>==---------------------------------------------------------------------------

const BasicBlock *Jump::get_dest() const {
    if (!has_parent())
        return nullptr;

    return get_parent()->get_succ(0);
}

void Jump::print(std::ostream &os, PrintPolicy policy) const {
    assert(policy != PrintPolicy::Use && "jump does not define a value!");

    if (policy == PrintPolicy::Def) {
        os << std::format("jump bb{}\n", get_dest()->position());
    }
}

//>==---------------------------------------------------------------------------
//                          Brif Implementation
//>==---------------------------------------------------------------------------

const BasicBlock* Brif::get_true_dest() const {
    if (!has_parent())
        return nullptr;

    return get_parent()->get_succ(0);
}

const BasicBlock* Brif::get_false_dest() const {
    if (!has_parent())
        return nullptr;

    return get_parent()->get_succ(1);
}

void Brif::print(std::ostream &os, PrintPolicy policy) const {
    assert(policy != PrintPolicy::Use && "brif does not define a value!");

    if (policy == PrintPolicy::Def) {
        os << "brif ";
        get_cond()->print(os, PrintPolicy::Use);
        os << std::format(" bb{} else bb{}\n", 
            get_true_dest()->position(), get_false_dest()->position());
    }
}

//>==---------------------------------------------------------------------------
//                          Phi Implementation
//>==---------------------------------------------------------------------------

void Phi::add_edge(Value *value, BasicBlock *pred) {
    assert(*value->get_type() == *m_type && 
        "incoming value does not match node type!");

    add_operand(value);
    m_preds.push_back(pred);
}

void Phi::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << std::format("%{}: {}", def(), get_type()->to_string());
    } else if (policy == PrintPolicy::Def) {
        os << std::format("%{} := phi <{}> ", m_def, get_type()->to_string());

        for (uint32_t i = 0, e = num_edges(); i < e; ++i) {
            const Edge edge = get_edge(i);

            os << '(';
            edge.value->print(os, PrintPolicy::Use);
            os << std::format(", bb{})", edge.pred->position());

            if (i + 1 != e)
                os << ", ";
        }
        
        os << "\n";
    }
}

//>==---------------------------------------------------------------------------
//                          Unop Implementation
//>==---------------------------------------------------------------------------

void Unop::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << std::format("%{}: {}", def(), get_type()->to_string());
    } else if (policy == PrintPolicy::Def) {
        os << std::format("%{} := ", def());

        switch (op()) {
            case Op::Not:
                os << "not";
                break;
            case Op::INeg:
                os << "ineg";
                break;
            case Op::FNeg:
                os << "fneg";
                break;
        }

        os << std::format(" <{}> ", get_type()->to_string());
        get_value()->print(os, PrintPolicy::Use);
        os << '\n';
    }
}

//>==---------------------------------------------------------------------------
//                          Binop Implementation
//>==---------------------------------------------------------------------------

void Binop::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << std::format("%{}: {}", def(), get_type()->to_string());
    } else if (policy == PrintPolicy::Def) {
        os << std::format("%{} := ", def());

        switch (op()) {
            case Op::IAdd:
                os << "iadd";
                break;
            case Op::FAdd:
                os << "fadd";
                break;
            case Op::ISub:
                os << "isub";
                break;
            case Op::FSub:
                os << "fsub";
                break;
            case Op::IMul:
                os << "imul";
                break;
            case Op::FMul:
                os << "fmul";
                break;
            case Op::SDiv:
                os << "sdiv";
                break;
            case Op::UDiv:
                os << "udiv";
                break;
            case Op::FDiv:
                os << "fdiv";
                break;
            case Op::SMod:
                os << "smod";
                break;
            case Op::UMod:
                os << "umod";
                break;
            case Op::And:
                os << "and";
                break;
            case Op::Or:
                os << "or";
                break;
            case Op::Xor:
                os << "xor";
                break;
            case Op::Shl:
                os << "shl";
                break;
            case Op::Shr:
                os << "shr";
                break;
            case Op::Sar:
                os << "sar";
                break;
        }

        os << std::format(" <{}> ", m_type->to_string());
        get_lhs()->print(os, PrintPolicy::Use);
        os << ", ";
        get_rhs()->print(os, PrintPolicy::Use);
        os << '\n';
    }
}

//>==---------------------------------------------------------------------------
//                          Cast Implementation
//>==---------------------------------------------------------------------------

void Cast::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << std::format("%{}: {}", m_def, m_type->to_string());
    } else if (policy == PrintPolicy::Def) {
        os << std::format("%{} := ", m_def);

        switch (kind()) {
            case Kind::SExt:
                os << "sext";
                break;
            case Kind::ZExt:
                os << "zext";
                break;
            case Kind::FExt:
                os << "fext";
                break;
            case Kind::ITrunc:
                os << "itrunc";
                break;
            case Kind::FTrunc:
                os << "ftrunc";
                break;
            case Kind::S2F:
                os << "s2f";
                break;
            case Kind::U2F:
                os << "u2f";
                break;
            case Kind::F2S:
                os << "f2s";
                break;
            case Kind::F2U:
                os << "f2u";
                break;
            case Kind::P2I:
                os << "p2i";
                break;
            case Kind::I2P:
                os << "i2p";
                break;
            case Kind::Reint:
                os << "reint";
                break;
        }

        os << std::format(" <{}> ", m_type->to_string());
        get_value()->print(os, PrintPolicy::Use);
        os << '\n';
    }
}

//>==---------------------------------------------------------------------------
//                          Cmp Implementation
//>==---------------------------------------------------------------------------

void Cmp::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << std::format("%{}: {}", m_def, m_type->to_string());
    } else if (policy == PrintPolicy::Def) {
        os << std::format("%{} := ", m_def);

        switch (pred()) {
            case Predicate::IEq:
                os << "ieq";
                break;
            case Predicate::FEq:
                os << "feq";
                break;
            case Predicate::INe:
                os << "ine";
                break;
            case Predicate::FNe:
                os << "fne";
                break;
            case Predicate::Slt:
                os << "slt";
                break;
            case Predicate::Ult:
                os << "ult";
                break;
            case Predicate::Flt:
                os << "flt";
                break;
            case Predicate::Sle:
                os << "sle";
                break;
            case Predicate::Ule:
                os << "ule";
                break;
            case Predicate::Fle:
                os << "fle";
                break;
            case Predicate::Sgt:
                os << "sgt";
                break;
            case Predicate::Ugt:
                os << "ugt";
                break;
            case Predicate::Fgt:
                os << "fgt";
                break;
            case Predicate::Sge:
                os << "sge";
                break;
            case Predicate::Uge:
                os << "uge";
                break;
            case Predicate::Fge:
                os << "fge";
                break;
        }

        os << std::format(" <{}> ", get_type()->to_string());
        get_lhs()->print(os, PrintPolicy::Use);
        os << ", ";
        get_rhs()->print(os, PrintPolicy::Use);
        os << '\n';
    }
}
