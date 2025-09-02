#ifndef STATIM_SIIR_SSA_REWRITE_PASS_HPP_
#define STATIM_SIIR_SSA_REWRITE_PASS_HPP_

#include "siir/basicblock.hpp"
#include "siir/instbuilder.hpp"
#include "siir/instruction.hpp"
#include "siir/local.hpp"
#include "siir/pass.hpp"

#include <unordered_map>
#include <vector>

namespace stm {
namespace siir {

class Instruction;
class Local;

/// Function-based pass to rewrite memory load/store operations into true SSA
/// instructions so that optimizations can be properly ran over locals.
///
/// This pass implements some of the algorithms outlined by Braun et al.
/// See: https://link.springer.com/chapter/10.1007/978-3-642-37051-9_6
class SSARewrite final : public Pass {
    using BlockDefs = std::unordered_map<Local*,
        std::unordered_map<BasicBlock*, Value*>>;

    InstBuilder m_builder;

    /// A mapping for each basic block between a local variable and a store
    /// instruction.
    BlockDefs m_block_defs = {};

    /// A list of instructions to remove after the current process.
    std::vector<Instruction*> m_to_remove = {};

    /// Process a function in the target graph.
    void process(Function* fn);

    /// Process a load instruction.
    void process_load(BasicBlock* blk, Instruction* inst);

    /// Process a store instruction.
    void process_store(BasicBlock* blk, Instruction* inst);

    /// Register a variable write (def).
    void write_variable(Local* var, BasicBlock* blk, Value* value);

    // Read the latest definition of |var| based on current block |blk|.
    Value* read_variable(Local* var, BasicBlock* blk);
    Value* read_variable_recursive(Local* var, BasicBlock* blk);

    /// Add the appropriate incoming values to |phi| based on the predecessors
    /// of block |blk|.
    Value* add_phi_operands(Local* var, BasicBlock* blk, Instruction* phi);

    /// Attempt to remove a phi instruction which could be considered trivial,
    /// i.e. merges less than two unique values. Returns the result of the 
    /// operation; the phi instruction or the distinguishable operand.
    Value* try_remove_trivial_phi(Local* var, Instruction* phi);

public:
    SSARewrite(CFG& cfg);

    void run() override;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_SSA_REWRITE_PASS_HPP_
