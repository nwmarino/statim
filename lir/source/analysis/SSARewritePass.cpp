//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/analysis/SSARewritePass.hpp"
#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Builder.hpp"
#include "lir/graph/CFG.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Instruction.hpp"
#include "lir/graph/Value.hpp"

#include <algorithm>
#include <functional>
#include <set>
#include <unordered_map>
#include <vector>

#ifdef LIR_SSA_DEBUGGING
#include <iostream>
#endif // LIR_SSA_DEBUGGING

using namespace lir;

/// Compute the reverse post order of flow for basic blocks in the given |func|.
static void compute_rpo(Function* func, std::vector<BasicBlock*>& rpo) {
    std::set<BasicBlock*> visited = {};
    std::vector<BasicBlock*> order = {};
    
    std::function<void(BasicBlock*)> dfs = [&](BasicBlock* block) {
        if (!visited.insert(block).second)
            return;

        for (BasicBlock* succ : block->get_succs())
            dfs(succ);
    
        order.push_back(block);
    };

    dfs(func->get_head());
    rpo.assign(order.rbegin(), order.rend());
}

void SSARewritePass::run() {
    m_builder.set_mode(Builder::InsertMode::Prepend);

    for (Function* func : m_cfg.get_functions())
        process(func);
}

void SSARewritePass::process(Function* func) {
    std::map<std::string, Local*> locals_copy = func->get_locals();
    for (const auto& [name, local] : locals_copy)
        promote_local(func, local);
}

void SSARewritePass::promote_local(Function* func, Local* local) {
#ifdef LIR_SSA_DEBUGGING
    std::cerr << "Promoting local: ";
    local->print(std::cerr);
    std::cerr << '\n';
#endif // LIR_SSA_DEBUGGING

    m_local = local;

    std::vector<BasicBlock*> rpo = {};
    compute_rpo(func, rpo);

    for (BasicBlock* block : rpo) {
        for (Instruction* inst = block->get_head(); inst; inst = inst->get_next()) {
            if (dynamic_cast<Load*>(inst) && inst->get_operand(0) == local) {
                // This instruction reads from |local|, meaning it should use 
                // the most recently defined value.
                Value* v = read_variable(block);
                inst->replace_all_uses_with(v);

                assert(!inst->used());

#ifdef LIR_SSA_DEBUGGING
                std::cerr << "[LOAD] replaced v" << inst->result_id() << 
                    " with ";
                v->print(std::cerr);
                std::cerr << std::endl;
#endif // LIR_SSA_DEBUGGING

                m_to_remove.push_back(inst);
            } else if (dynamic_cast<Store*>(inst) && inst->get_operand(1) == local) {
                // This instruction writes to |local|, meaning it defines a
                // new value.
                write_variable(block, inst->get_operand(0));
                m_to_remove.push_back(inst);
            }
        }

        m_visited.push_back(block);

        // For each basic block, if all of its predecessors have been visited,
        // and it is not already sealed, then seal the block.
        for (BasicBlock* block : rpo) {
            if (is_sealed(block))
                continue;

            bool all_preds_visited = true;
            for (BasicBlock* pred : block->get_preds()) {
                if (!visited(pred)) 
                    all_preds_visited = false;
            }

            if (all_preds_visited && !is_sealed(block))
                seal_block(block);
        }
    }

    for (Instruction* inst : m_to_remove) {
        assert(!inst->used() && "instruction is still in use!");
        inst->detach();
        delete inst;
    }

    if (!m_local->used()) {
        m_local->detach();
        delete m_local;
    }

    m_local = nullptr;
    m_sealed.clear();
    m_visited.clear();
    m_incomplete_phis.clear();
    m_to_remove.clear();
    m_current_def.clear();
}

void SSARewritePass::write_variable(BasicBlock* block, Value* value) {
    m_current_def[block] = value;
}

Value* SSARewritePass::read_variable(BasicBlock* block) {
    if (m_current_def.count(block) == 1)
        return m_current_def[block];

    return read_variable_recursive(block);
}

Value* SSARewritePass::add_phi_operands(Phi* phi) {
    assert(phi->num_operands() == 0 && "phi already has operands!");

    // For each predecessor to the parent of |phi|, try and read a def of 
    // |local| and add it as an incoming edge to |phi|.
    for (BasicBlock* pred : phi->get_parent()->get_preds()) {
        Value* value = read_variable(pred);

#ifdef LIR_SSA_DEBUGGING
        std::cerr << "[PHI bb" << phi->get_parent()->get_number() << "] v" 
            << phi->result_id() << " new operand: ";
        value->print(std::cerr);
        std::cerr << '\n';
#endif // LIR_SSA_DEBUGGING

        phi->add_edge(value, pred);
    }

    return try_remove_trivial_phi(phi);
}

Value* SSARewritePass::read_variable_recursive(BasicBlock* block) {
    assert(!block->is_entry() && block->num_preds() > 0);

    if (!is_sealed(block)) {
        m_builder.set_insert(block);
        Phi* phi = m_builder.build_phi(m_local->get_allocated_type());
        if (m_incomplete_phis.count(block) == 0)
            m_incomplete_phis.emplace(block, std::unordered_map<Local*, std::vector<Phi*>>());

        if (m_incomplete_phis[block].count(m_local))
            m_incomplete_phis[block].emplace(m_local, std::vector<Phi*>());

        m_incomplete_phis[block][m_local].push_back(phi);
        write_variable(block, phi);
        return phi;
    } else if (block->num_preds() == 1) {
        // Only one predecessor to the block, so we can recursively look in the 
        // predecessor for a def.
        Value* v = read_variable(block->get_preds()[0]);
        m_current_def[block] = v;
        return v;
    }

    // There are multiple predecessors to |block|, so there may be multiple 
    // incoming defs, which means a phi function is necessary for now.

    m_builder.set_insert(block);

    Phi* phi = m_builder.build_phi(m_local->get_allocated_type());
    m_current_def[block] = phi;

    Value* v = add_phi_operands(phi);
    m_current_def[block] = v;

    return v;
}

Value* SSARewritePass::try_remove_trivial_phi(Phi* phi) {
    // For each of |phi|'s edges, see if it is a reference to the phi itself or 
    // one of its operand to determine if it is considered trivial.
    //
    // Note that since a phi node just propogates values as they step through
    // control flow, a unique operand can replace all uses of the phi thereof.
    Value* same = nullptr;
    for (uint32_t i = 0; i < phi->num_edges(); ++i) {
        Phi::Edge edge = phi->get_edge(i);

        if (edge.value == same || edge.value == phi) {
            // This is a reference to one of the phi's operands or a reference
            // to the phi itself.
            continue;
        }

        if (same != nullptr) {
            // This phi merges at least unique two values, so it is not trivial.
            return phi;
        }

        same = edge.value;
    }

    assert(same);

    // Track each user of |phi|, which is not |phi| itself. This is to record
    // a copy for later.
    std::vector<User*> users = {};
    for (Use* use : phi->uses()) {
        if (use->get_user() != phi)
            users.push_back(use->get_user());
    }

    // Replace all uses of |phi| with it's unique value.
    phi->replace_all_uses_with(same);

    // If |phi| is being used as the current definition for the local being
    // processed, replace it with the only unique operand.
    for (auto& [block, def] : m_current_def) {
        if (def == phi)
            m_current_def[block] = same;
    }

    phi->detach();
    delete phi;

    for (User* user : users) {
        Instruction* inst = dynamic_cast<Instruction*>(user);
        if (!user)
            continue;

        Phi* phi_user = dynamic_cast<Phi*>(inst);
        if (phi_user)
            try_remove_trivial_phi(phi_user);
    }

    return same;
}

bool SSARewritePass::visited(BasicBlock* block) const {
    return std::find(
        m_visited.begin(), m_visited.end(), block) != m_visited.end();
}

bool SSARewritePass::is_sealed(BasicBlock* block) const {
    return std::find(
        m_sealed.begin(), m_sealed.end(), block) != m_sealed.end();
}

void SSARewritePass::seal_block(BasicBlock* block) {
    assert(!is_sealed(block) && "block is already sealed!");

    // For each incomplete phi in |block|, attach its operands.
    for (auto& [local, phis] : m_incomplete_phis[block]) {
        for (Phi* phi : phis)
            add_phi_operands(phi);

        phis.clear();
    }

    m_sealed.push_back(block);
    
#ifdef LIR_SSA_DEBUGGING
    std::cerr << "Sealed block: bb" << block->get_position() << "\n";
#endif // LIR_SSA_DEBUGGING
}
