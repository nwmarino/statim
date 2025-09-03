#include "siir/trivial_dce_pass.hpp"

using namespace stm;
using namespace stm::siir;

void TrivialDCEPass::run() {
    for (auto fn : m_cfg.functions())
        process(fn);
}

void TrivialDCEPass::process(Function* fn) {
    for (auto blk = fn->front(); blk; blk = blk->next()) {
        for (auto inst = blk->front(); inst; inst = inst->next()) {
            if (inst->is_trivially_dead())
                m_to_remove.push_back(inst);
        }
    }

    for (auto inst : m_to_remove) {
        inst->detach_from_parent();
        delete inst;
    }

    m_to_remove.clear();
}
