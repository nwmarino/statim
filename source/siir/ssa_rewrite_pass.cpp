#include "siir/basicblock.hpp"
#include "siir/cfg.hpp"
#include "siir/ssa_rewrite_pass.hpp"
#include "siir/instbuilder.hpp"
#include "siir/instruction.hpp"
#include "siir/value.hpp"

using namespace stm;
using namespace stm::siir;

SSARewrite::SSARewrite(CFG& cfg) : Pass(cfg), m_builder(cfg) {}

void SSARewrite::run() {
    m_builder.set_insert_mode(InstBuilder::Prepend);

    for (auto& fn : m_cfg.functions())
        process(fn);
}

void SSARewrite::process(Function* fn) {
    // For each basic block in a function, process every load and store
    // instruction within the block.
    for (auto blk = fn->front(); blk; blk = blk->next()) {
        for (auto inst = blk->front(); inst; inst = inst->next()) {
            if (inst->is_load()) {
                process_load(blk, inst);
            } else if (inst->is_store()) {
                process_store(blk, inst);
            }
        }

        // Finally mutate the block and apply destructing changes.
        for (auto& inst : m_to_remove) {
            inst->detach_from_parent();
            delete inst;
        }

        m_to_remove.clear();
    }

    m_block_defs.clear();
}

void SSARewrite::process_load(BasicBlock* blk, Instruction* inst) {
    if (Local* local = dynamic_cast<Local*>(inst->get_operand(0))) {
        Value* value = read_variable(local, blk);
        inst->replace_all_uses_with(value);
        m_to_remove.push_back(inst);
    }
}

void SSARewrite::process_store(BasicBlock* blk, Instruction* inst) {
    if (Local* local = dynamic_cast<Local*>(inst->get_operand(1))) {
        write_variable(local, blk, inst->get_operand(0));
        m_to_remove.push_back(inst);
    }
}

void SSARewrite::write_variable(Local* var, BasicBlock* blk, Value* value) {
    if (m_block_defs.find(var) == m_block_defs.end())
        m_block_defs[var] = std::unordered_map<BasicBlock*, Value*>();

    m_block_defs[var][blk] = value;
}

Value* SSARewrite::read_variable(Local* var, BasicBlock* blk) {
    if (m_block_defs.count(var) == 1) {
        if (m_block_defs[var].count(blk) == 1)
            // Local value numbering.
            return m_block_defs[var][blk];
    }

    // Global value numbering.
    return read_variable_recursive(var, blk);
}

Value* SSARewrite::add_phi_operands(Local* var, BasicBlock* blk, 
                                    Instruction* phi) {
    assert(phi->num_operands() == 0);

    // For each predecessor to |blk|, which is the block that |phi| is in, try 
    // and read a def of |local| and add it as an incoming edge to |phi|.
    for (auto& pred : blk->preds())
        phi->add_incoming(m_cfg, read_variable(var, pred), pred);
    
    return try_remove_trivial_phi(var, phi);
}

Value* SSARewrite::read_variable_recursive(Local* var, BasicBlock* blk) {
    Value* val = nullptr;
    if (blk->num_preds() == 1) {
        // Only one predecessor to the block, so we can try to recursively look 
        // in the predecessor for a def.
        val = read_variable(var, blk->preds()[0]);
    } else {
        // There are multiple predecessors to |blk|, so there may be ultiple 
        // incoming defs, which means a phi node is likely necessary.
        //m_builder.set_insert(blk);
        m_builder.set_insert(blk);
        Instruction* phi = m_builder.build_phi(var->get_allocated_type());
        val = phi;
        write_variable(var, blk, val);
        val = add_phi_operands(var, blk, phi);
    }

    write_variable(var, blk, val);
    return val;
}

Value* SSARewrite::try_remove_trivial_phi(Local* var, Instruction* phi) {
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

    // The logic in the loop above asserts that the phi is considered trivial.
    // This means it's unnecessary and we can replace all uses of the phi to
    // that of the (repeated) operand.

    std::vector<User*> phi_users;
    for (auto* use : phi->uses())
        if (use->get_user() != phi)
            phi_users.push_back(use->get_user());

    phi->replace_all_uses_with(same);
    phi->detach_from_parent();

    // If the |phi| is the latest def in any block, replace it with |same|.
    for (auto& [ block, def ] : m_block_defs[var]) {
        if (def == phi)
            m_block_defs[var][block] = same;
    }

    delete phi;

    for (auto& user : phi_users)
        if (auto* instr = dynamic_cast<Instruction*>(user))
            if (instr->is_phi())
                try_remove_trivial_phi(var, instr);

    return same;
}
