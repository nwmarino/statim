#ifndef STATIM_SIIR_LLVM_TRANSLATE_PASS_H_
#define STATIM_SIIR_LLVM_TRANSLATE_PASS_H_

#include "core/stmc.hpp"

#ifdef STMC_LLVM_SUPPORT

#include "siir/basicblock.hpp"
#include "siir/instruction.hpp"
#include "siir/inlineasm.hpp"
#include "siir/pass.hpp"

#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
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
    llvm::Module& m_module;
    llvm::LLVMContext* m_context;
    std::unique_ptr<llvm::IRBuilder<>> m_builder = nullptr;
    std::unordered_map<Global*, llvm::GlobalVariable*> m_globals = {};
    std::unordered_map<Function*, llvm::Function*> m_functions = {};
    std::unordered_map<Local*, llvm::AllocaInst*> m_locals = {};
    std::unordered_map<BasicBlock*, llvm::BasicBlock*> m_blocks = {};
    std::unordered_map<Instruction*, llvm::Value*> m_insts = {};
    std::unordered_map<std::string, llvm::GlobalVariable*> m_strings = {};
    std::unordered_map<InlineAsm*, llvm::InlineAsm*> m_iasm = {};
    std::vector<std::pair<Instruction*, llvm::PHINode*>> m_delayed_phis = {};

    llvm::Type* translate(const Type* ty);
    llvm::GlobalVariable* translate(Global* global);
    llvm::Function* translate(Function* fn);
    llvm::Argument* translate(Argument* arg);
    llvm::AllocaInst* translate(Local* local);
    llvm::BasicBlock* translate(BasicBlock* blk);
    llvm::Value* translate(Instruction* inst);
    llvm::Constant* translate(Constant* constant);
    llvm::InlineAsm* translate(InlineAsm* iasm);
    llvm::Value* translate(Value* value);

    void convert(StructType* type);
    void convert(Global* global);
    void convert(Function* fn);
    void convert(BasicBlock* bb);
    void convert(Instruction* inst);
    void convert(InlineAsm* iasm);

public:
    LLVMTranslatePass(CFG& cfg, llvm::Module& module) 
            : Pass(cfg), m_module(module) {
        m_context = &m_module.getContext();
    }

    void run() override;
};

} // namespace stm::siir

#endif // STMC_LLVM_SUPPORT

#endif // STATIM_SIIR_LLVM_TRANSLATE_PASS_H_
