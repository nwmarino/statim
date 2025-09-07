#ifndef STATIM_SIIR_LLVM_TRANSLATE_PASS_H_
#define STATIM_SIIR_LLVM_TRANSLATE_PASS_H_

#include "siir/basicblock.hpp"
#include "siir/instruction.hpp"
#include "siir/pass.hpp"

#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include <cassert>
#include <unordered_map>

namespace stm::siir {

/// Graph-wide pass that translate an SIIR control flow graph object into the
/// equivelant LLVM IR module.
///
/// The LLVM target will be derived from the existing target of the SIIR graph.
class LLVMTranslatePass final : public Pass {
    std::unique_ptr<llvm::Module> m_module = nullptr;
    std::unique_ptr<llvm::LLVMContext> m_context = nullptr;
    std::unique_ptr<llvm::IRBuilder<>> m_builder = nullptr;
    std::unordered_map<Global*, llvm::GlobalVariable*> m_globals = {};
    std::unordered_map<Function*, llvm::Function*> m_functions = {};
    std::unordered_map<Local*, llvm::AllocaInst*> m_locals = {};
    std::unordered_map<BasicBlock*, llvm::BasicBlock*> m_blocks = {};
    std::unordered_map<Instruction*, llvm::Instruction*> m_insts = {};
    std::vector<std::pair<Instruction*, llvm::PHINode*>> m_delayed_phis = {};

    llvm::Type* translate(const Type* ty);
    llvm::GlobalVariable* translate(Global* global);
    llvm::Function* translate(Function* fn);
    llvm::Argument* translate(Argument* arg);
    llvm::AllocaInst* translate(Local* local);
    llvm::BasicBlock* translate(BasicBlock* blk);
    llvm::Instruction* translate(Instruction* inst);
    llvm::Constant* translate(Constant* constant);
    llvm::Value* translate(Value* value);

    void convert(Global* global);
    void convert(Function* fn);
    void convert(BasicBlock* bb);
    void convert(Instruction* inst);

public:
    LLVMTranslatePass(CFG& cfg) : Pass(cfg) {}

    void run() override;

    /// Returns the module that results from a run of this translation pass.
    std::unique_ptr<llvm::Module> module() { 
        assert(m_module && "translation pass has not occured!");    
        return std::move(m_module); 
    }
};

} // namespace stm::siir

#endif // STATIM_SIIR_LLVM_TRANSLATE_PASS_H_
