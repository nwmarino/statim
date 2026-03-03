//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/analysis/ConstantFolding.h"
#include "lir/graph/Constant.h"
#include "lir/graph/Instruction.h"

using namespace lir;

void ConstantFolding::run() {
    for (Function* func : m_cfg.get_functions())
        process(func);
}

void ConstantFolding::process(Function* func) {
    m_to_remove.clear();

    BasicBlock* block = func->get_head();
    while (block) {
        for (Instruction* inst = block->get_head(); inst; inst = inst->get_next()) {
            if (Binop* binop = dynamic_cast<Binop*>(inst)) {
                process(binop);
            } else if (Unop* unop = dynamic_cast<Unop*>(inst)) {
                process(unop);
            } else if (Cmp* cmp = dynamic_cast<Cmp*>(inst)) {
                process(cmp);
            }
        }

        block = block->get_next();
    }

    for (Instruction* inst : m_to_remove) {
        inst->detach();
        delete inst;
    }
}

void ConstantFolding::process(Binop* op) {
    switch (op->op()) 
    {
    case Binop::Op::IAdd: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() + rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::ISub: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() - rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::IMul: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() * rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::SDiv:
    case Binop::Op::UDiv: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() / rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }
    
    case Binop::Op::SMod:
    case Binop::Op::UMod: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() % rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::FAdd: {
        Float* lhs = dynamic_cast<Float*>(op->get_lhs());
        Float* rhs = dynamic_cast<Float*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Float::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() + rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::FSub: {
        Float* lhs = dynamic_cast<Float*>(op->get_lhs());
        Float* rhs = dynamic_cast<Float*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Float::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() - rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::FMul: {
        Float* lhs = dynamic_cast<Float*>(op->get_lhs());
        Float* rhs = dynamic_cast<Float*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Float::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() * rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::FDiv: {
        Float* lhs = dynamic_cast<Float*>(op->get_lhs());
        Float* rhs = dynamic_cast<Float*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Float::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() / rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::And: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() & rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::Or: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() | rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::Xor: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() ^ rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::Shl: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() << rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Binop::Op::Shr:
    case Binop::Op::Sar: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() >> rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    default:
        break;
    }
}

void ConstantFolding::process(Unop* op) {
    switch (op->op()) 
    {
    case Unop::Op::Not: {
        Integer* val = dynamic_cast<Integer*>(op->get_value());
        if (!val)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            ~val->get_value()
        ));

        m_to_remove.push_back(op);
        break;
    }

    case Unop::Op::INeg: {
        Integer* val = dynamic_cast<Integer*>(op->get_value());
        if (!val)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            -val->get_value()
        ));

        m_to_remove.push_back(op);
        break;
    }

    case Unop::Op::FNeg: {
        Float* val = dynamic_cast<Float*>(op->get_value());
        if (!val)
            break;

        op->replace_all_uses_with(lir::Float::get(
            m_cfg, 
            op->get_type(), 
            -val->get_value()
        ));

        m_to_remove.push_back(op);
        break;
    }

    default:
        break;
    }
}

void ConstantFolding::process(Cmp* op) {
    switch (op->pred())
    {
    case Cmp::Predicate::IEq: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() == rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }
        
    case Cmp::Predicate::FEq: {
        Float* lhs = dynamic_cast<Float*>(op->get_lhs());
        Float* rhs = dynamic_cast<Float*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Float::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() == rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::INe: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() != rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::FNe: {
        Float* lhs = dynamic_cast<Float*>(op->get_lhs());
        Float* rhs = dynamic_cast<Float*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Float::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() != rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Slt: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() < rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Ult: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            (uint64_t)(lhs->get_value()) != (uint64_t)(rhs->get_value()))
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Flt: {
        Float* lhs = dynamic_cast<Float*>(op->get_lhs());
        Float* rhs = dynamic_cast<Float*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Float::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() < rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Sle: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() <= rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Ule: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            (uint64_t)(lhs->get_value()) <= (uint64_t)(rhs->get_value()))
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Fle: {
        Float* lhs = dynamic_cast<Float*>(op->get_lhs());
        Float* rhs = dynamic_cast<Float*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Float::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() <= rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Sgt: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() > rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Ugt: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            (uint64_t)(lhs->get_value()) > (uint64_t)(rhs->get_value()))
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Fgt: {
        Float* lhs = dynamic_cast<Float*>(op->get_lhs());
        Float* rhs = dynamic_cast<Float*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Float::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() > rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Sge: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() >= rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Uge: {
        Integer* lhs = dynamic_cast<Integer*>(op->get_lhs());
        Integer* rhs = dynamic_cast<Integer*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Integer::get(
            m_cfg, 
            op->get_type(), 
            (uint64_t)(lhs->get_value()) >= (uint64_t)(rhs->get_value()))
        );

        m_to_remove.push_back(op);
        break;
    }

    case Cmp::Predicate::Fge: {
        Float* lhs = dynamic_cast<Float*>(op->get_lhs());
        Float* rhs = dynamic_cast<Float*>(op->get_rhs());

        if (!lhs || !rhs)
            break;

        op->replace_all_uses_with(lir::Float::get(
            m_cfg, 
            op->get_type(), 
            lhs->get_value() >= rhs->get_value())
        );

        m_to_remove.push_back(op);
        break;
    }

    default:
        break;
    }
}
