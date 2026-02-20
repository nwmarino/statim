//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_SSA_REWRITE_PASS_H_
#define LIR_SSA_REWRITE_PASS_H_

#include "lir/analysis/Pass.hpp"
#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Builder.hpp"
#include "lir/graph/Instruction.hpp"
#include "lir/graph/Local.hpp"

#include <unordered_map>
#include <vector>

namespace lir {

class Instruction;
class Local;

/// Function-based pass to rewrite memory load/store operations into true SSA
/// instructions so that optimizations can be properly ran over locals.
///
/// This pass implements some of the algorithms outlined by Braun et al.
/// See: https://link.springer.com/chapter/10.1007/978-3-642-37051-9_6
class SSARewritePass final : public Pass {
    Builder m_builder;

    /// The current local being processed.
    Local* m_local = nullptr;

    std::unordered_map<BasicBlock*, Value*> m_current_def = {};

    std::unordered_map<BasicBlock*, std::unordered_map<Local*, 
        std::vector<Phi*>>> m_incomplete_phis = {};

    /// A list of instructions to remove after the current process.
    std::vector<Instruction*> m_to_remove = {};

    std::vector<BasicBlock*> m_visited = {};

    std::vector<BasicBlock*> m_sealed = {};

    /// Perform an SSA rewrite for the given |func|.
    void process(Function* func);

    void promote_local(Function* func, Local* local);

    /// Register a variable write (def) for the given |value| in |block|.
    void write_variable(BasicBlock* block, Value* value);

    // Read the latest definition of the target local for the given |block|.
    Value* read_variable(BasicBlock* block);
    Value* read_variable_recursive(BasicBlock* block);

    Value* add_phi_operands(Phi* phi);

    /// Attempt to remove a phi instruction which could be considered trivial,
    /// i.e. merges less than two unique values. 
    /// Returns the result of the operation; the phi instruction or the 
    /// distinguishable operand.
    Value* try_remove_trivial_phi(Phi* phi);

    /// Mark the given |block| as having been visited.
    bool visited(BasicBlock *block) const;

    /// Test if the given |block| is sealed.
    bool is_sealed(BasicBlock *block) const;

    /// Mark the given |block| as being sealed.
    void seal_block(BasicBlock *block);

public:
    SSARewritePass(CFG &cfg) : Pass(cfg), m_builder(cfg) {}

    ~SSARewritePass() = default;

    SSARewritePass(const SSARewritePass&) = delete;
    void operator=(const SSARewritePass&) = delete;

    SSARewritePass(SSARewritePass&&) noexcept = delete;
    void operator=(SSARewritePass&&) noexcept = delete;

    void run() override;
};

} // namespace lir

#endif // LIR_SSA_REWRITE_PASS_H_
