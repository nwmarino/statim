#include "siir/basicblock.hpp"
#include "siir/cfg.hpp"
#include "siir/ssa_rewrite_pass.hpp"
#include "siir/constant.hpp"
#include "siir/instbuilder.hpp"
#include "siir/instruction.hpp"
#include "siir/value.hpp"
#include "tree/decl.hpp"
#include <algorithm>
#include <functional>
#include <set>
#include <unordered_map>
#include <vector>

using namespace stm;
using namespace stm::siir;

#define SSAR_DEBUGGING 1

static void compute_rpo(Function* fn, std::vector<BasicBlock*>& rpo) {
    std::set<BasicBlock*> visited;
    std::vector<BasicBlock*> order;
    
    std::function<void(BasicBlock*)> dfs = [&](BasicBlock* blk) {
        if (!visited.insert(blk).second)
            return;

        for (auto* succ : blk->succs())
            dfs(succ);
    
        order.push_back(blk);
    };

    dfs(fn->front());
    rpo.assign(order.rbegin(), order.rend());
}

SSARewrite::SSARewrite(CFG& cfg) : Pass(cfg), m_builder(cfg) {}

void SSARewrite::run() {
    m_builder.set_insert_mode(InstBuilder::Prepend);

    for (auto& fn : m_cfg.functions())
        process(fn);
}

void SSARewrite::process(Function* fn) {
    for (auto& [name, local] : fn->locals()) {
        promote_local(fn, local);
    }
}

void SSARewrite::promote_local(Function* fn, Local* local) {
    m_local = local;

    std::vector<BasicBlock*> rpo;
    compute_rpo(fn, rpo);

    for (auto* blk : rpo) {
        for (auto* inst = blk->front(); inst; inst = inst->next()) {
            if (inst->is_load() && inst->get_operand(0) == local) {
                // This instruction reads from |local|, meaning it uses the
                // the most recent value.
                Value* v = read_variable(blk);
                inst->replace_all_uses_with(v);
                assert(!inst->used());

#ifdef SSAR_DEBUGGING
                std::cerr << "[LOAD] replaced v" << inst->result_id() << 
                    " with ";
                v->print(std::cerr);
                std::cerr << std::endl;
#endif // SSAR_DEBUGGING

                m_to_remove.push_back(inst);
            } else if (inst->is_store() && inst->get_operand(1) == local) {
                // This instruction writes to |local|, meaning it defines a
                // new value.
                write_variable(blk, inst->get_operand(0));
                m_to_remove.push_back(inst);
            }
        }
    }

    for (auto* blk : rpo) {
        if (blk->num_preds() > 1) {
            // Check if any definition from a predecessor is missing
            auto blk_pos = std::find(rpo.begin(), rpo.end(), blk);
            for (auto* pred : blk->preds()) {
                auto pred_pos = std::find(rpo.begin(), rpo.end(), pred);
                if (pred_pos > blk_pos) {
                    // This is a back-edge (pred comes after blk in RPO)
                    read_variable(blk);
                    break;
                }
            }
        }
    }

    for (auto& inst : m_to_remove) {
        assert(!inst->used());
        inst->detach_from_parent();
        delete inst;
    }

    m_to_remove.clear();
    m_current_def.clear();
}

void SSARewrite::write_variable(BasicBlock* blk, Value* value) {
    m_current_def[blk] = value;
}

Value* SSARewrite::read_variable(BasicBlock* blk) {
    if (m_current_def.count(blk) == 1)
        return m_current_def[blk];

    return read_variable_recursive(blk);
}

Value* SSARewrite::add_phi_operands(Instruction* phi) {
    assert(phi->num_operands() == 0);

    // For each predecessor to |blk|, which is the block that |phi| is in, try 
    // and read a def of |local| and add it as an incoming edge to |phi|.
    for (auto& pred : phi->get_parent()->preds()) {
        Value* value = read_variable(pred);

#ifdef SSAR_DEBUGGING
        std::cerr << "[PHI bb" << phi->get_parent()->get_number() << "] v" << 
            phi->result_id() << " new operand: ";
        value->print(std::cerr);
        std::cerr << '\n';
#endif // SSAR_DEBUGGING

        phi->add_incoming(m_cfg, value, pred);
    }

    return try_remove_trivial_phi(phi);
}

Value* SSARewrite::read_variable_recursive(BasicBlock* blk) {
    assert(!blk->is_entry_block() && blk->num_preds() > 0);

    if (blk->num_preds() == 1) {
        // Only one predecessor to the block, so we can recursively look in the 
        // predecessor for a def.
        Value* v = read_variable(blk->preds()[0]);
        m_current_def[blk] = v;
        return v;
    }

    // There are multiple predecessors to |blk|, so there may be multiple 
    // incoming defs, which means a phi function is necessary for now.
    m_builder.set_insert(blk);
    Instruction* phi = m_builder.build_phi(m_local->get_allocated_type());
    m_current_def[blk] = phi;
    Value* v = add_phi_operands(phi);
    m_current_def[blk] = v;
    return v;
}

Value* SSARewrite::try_remove_trivial_phi(Instruction* phi) {
    // For each incoming value to |phi|, see if it is a reference to the phi
    // itself or another operand to determine if it is considered trivial.
    // A phi can also be considered trivial if it merges less than two unique
    // values.
    Value* same = nullptr;
    for (auto op : phi->get_operand_list()) {
        PhiOperand* phi_op = dynamic_cast<PhiOperand*>(op->get_value());
        assert(phi_op && "non phi-compatible operand in phi operand list");

        if (phi_op->get_value() == same || phi_op->get_value() == phi) {
            // This is a reference to one of the phi's operands or a reference
            // to the phi itself.
            continue;
        }

        if (same) {
            // This phi merges at least unique two values, so it is not trivial.
            return phi;
        }

        same = phi_op->get_value();
    }

    assert(same);

    std::vector<User*> phi_users;
    for (auto* use : phi->uses())
        if (use->get_user() != phi)
            phi_users.push_back(use->get_user());

    phi->replace_all_uses_with(same);

    for (auto& [blk, def] : m_current_def)
        if (def == phi)
            m_current_def[blk] = same;

    phi->detach_from_parent();
    assert(!phi->used());
    delete phi;

    for (auto& user : phi_users)
        if (auto* instr = dynamic_cast<Instruction*>(user))
            if (instr->is_phi())
                try_remove_trivial_phi(instr);

    return same;
}
