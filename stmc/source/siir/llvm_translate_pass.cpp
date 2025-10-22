#include "core/stmc.hpp"

#ifdef STMC_LLVM_SUPPORT
#include "siir/llvm_translate_pass.hpp"
#include "siir/constant.hpp"
#include "siir/function.hpp"
#include "siir/inlineasm.hpp"
#include "siir/instruction.hpp"
#include "siir/type.hpp"

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/FPEnv.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/Casting.h"
#include "llvm/Target/TargetMachine.h"

#include <cassert>
#include <string>

using namespace stm;
using namespace stm::siir;

void LLVMTranslatePass::run() {
    m_builder = std::make_unique<llvm::IRBuilder<>>(*m_context);

    std::vector<StructType*> structs = m_cfg.structs();
    for (auto& type : structs)
        llvm::StructType::create(*m_context, type->get_name());

    for (auto& type : structs) 
        convert(type);

    for (auto& global : m_cfg.globals()) {
        llvm::GlobalVariable::LinkageTypes linkage;
        switch (global->get_linkage()) {
        case Global::LINKAGE_INTERNAL:
            linkage = llvm::GlobalVariable::InternalLinkage;
            break;
        case Global::LINKAGE_EXTERNAL:
            linkage = llvm::GlobalVariable::ExternalLinkage;
            break;
        }

        llvm::GlobalVariable* GV = new llvm::GlobalVariable(
            translate(static_cast<const PointerType*>(
                global->get_type())->get_pointee()),
            global->is_read_only(), 
            linkage,
            nullptr,
            global->get_name());

        m_module.insertGlobalVariable(GV);
        m_globals.emplace(global, GV);
    }

    for (auto& fn : m_cfg.functions()) {
        llvm::FunctionType* type = 
        llvm::dyn_cast<llvm::FunctionType>(translate(fn->get_type()));

        llvm::Function::LinkageTypes linkage;
        switch (fn->get_linkage()) {
        case Function::LINKAGE_INTERNAL:
            linkage = llvm::Function::InternalLinkage;
            break;
        case Function::LINKAGE_EXTERNAL:
            linkage = llvm::Function::ExternalLinkage;
            break;
        }

        llvm::Function* F = llvm::Function::Create(
            type, llvm::Function::ExternalLinkage, fn->get_name(), m_module);
        m_functions.emplace(fn, F);
    }

    for (auto& global : m_cfg.globals()) {
        convert(global);
    }

    for (auto& fn : m_cfg.functions()) {
        convert(fn);
    }

    assert(!llvm::verifyModule(m_module, &llvm::outs()));
}

llvm::Type* LLVMTranslatePass::translate(const Type* ty) {
    if (!ty)
        return llvm::Type::getVoidTy(*m_context);

    switch (ty->get_kind()) {
    case Type::TK_Int1:
        return llvm::Type::getInt1Ty(*m_context);
    case Type::TK_Int8:
        return llvm::Type::getInt8Ty(*m_context);
    case Type::TK_Int16:
        return llvm::Type::getInt16Ty(*m_context); 
    case Type::TK_Int32:
        return llvm::Type::getInt32Ty(*m_context);
    case Type::TK_Int64:
        return llvm::Type::getInt64Ty(*m_context);
    case Type::TK_Float32:
        return llvm::Type::getFloatTy(*m_context);
    case Type::TK_Float64:
        return llvm::Type::getDoubleTy(*m_context);
    case Type::TK_Array: {
        const auto* AT = static_cast<const ArrayType*>(ty);
        return llvm::ArrayType::get(
            translate(AT->get_element_type()), AT->get_size());
    }

    case Type::TK_Function: {
        const auto* FT = static_cast<const FunctionType*>(ty);
        std::vector<llvm::Type*> arg_types(FT->num_args(), nullptr);
        for (u32 idx = 0, e = FT->num_args(); idx != e; ++idx)
            arg_types[idx] = translate(FT->get_arg(idx));

        return llvm::FunctionType::get(
            translate(FT->get_return_type()), arg_types, false);
    }

    case Type::TK_Pointer: {
        return llvm::PointerType::getUnqual(
            translate(static_cast<const PointerType*>(ty)->get_pointee()));
    }

    case Type::TK_Struct: {
        return llvm::StructType::getTypeByName(
            *m_context, static_cast<const StructType*>(ty)->get_name());
    }

    default:
        assert(false && "no LLVM equivelant for SIIR type!");
    }
}

llvm::GlobalVariable* LLVMTranslatePass::translate(Global* global) {
    assert(m_globals.count(global) == 1);
    return m_globals[global];
}

llvm::Function* LLVMTranslatePass::translate(Function* fn) {
    assert(m_functions.count(fn) == 1);
    return m_functions[fn];
}

llvm::Argument* LLVMTranslatePass::translate(Argument* arg) {
    assert(arg->get_parent() && "argument does not have a parent!");
    auto fn = translate(arg->get_parent());
    assert(fn);
    return fn->getArg(arg->get_number());
}

llvm::AllocaInst* LLVMTranslatePass::translate(Local* local) {
    assert(m_locals.count(local) == 1);
    return m_locals[local];
}

llvm::BasicBlock* LLVMTranslatePass::translate(BasicBlock* blk) {
    assert(m_blocks.count(blk) == 1);
    return m_blocks[blk];
}

llvm::Value* LLVMTranslatePass::translate(Instruction* inst) {
    assert(m_insts.count(inst) == 1);
    return m_insts[inst];
}

llvm::Constant* LLVMTranslatePass::translate(Constant* constant) {
    if (auto CI = dynamic_cast<ConstantInt*>(constant)) {
        return llvm::ConstantInt::get(
            translate(CI->get_type()), CI->get_value());
    } else if (auto CFP = dynamic_cast<ConstantFP*>(constant)) {
        return llvm::ConstantFP::get(
            translate(CFP->get_type()), CFP->get_value());
    } else if (auto CN = dynamic_cast<ConstantNull*>(constant)) {
        return llvm::ConstantPointerNull::get(
            llvm::dyn_cast<llvm::PointerType>(translate(CN->get_type())));
    } else if (auto BA = dynamic_cast<BlockAddress*>(constant)) {
        return llvm::BlockAddress::get(translate(BA->get_block()));
    } else if (auto G = dynamic_cast<Global*>(constant)) {
        return translate(G);
    }

    assert(false && "no LLVM equivelant for SIIR constant!");
}

llvm::InlineAsm* LLVMTranslatePass::translate(InlineAsm* iasm) {
    assert(m_iasm.count(iasm) == 1);
    return m_iasm[iasm];
}

llvm::Value* LLVMTranslatePass::translate(Value* value) {
    if (auto A = dynamic_cast<Argument*>(value)) {
        return translate(A);
    } else if (auto C = dynamic_cast<Constant*>(value)) {
        return translate(C);
    } else if (auto F = dynamic_cast<Function*>(value)) {
        return translate(F);
    } else if (auto I = dynamic_cast<Instruction*>(value)) {
        return translate(I);
    } else if (auto IA = dynamic_cast<InlineAsm*>(value)) {
        return translate(IA);
    } else if (auto L = dynamic_cast<Local*>(value)) {
        return translate(L);
    }

    assert(false && "no LLVM equivelant for SIIR value!");
}

void LLVMTranslatePass::convert(StructType* type) {
    llvm::StructType* llvm_type = llvm::StructType::getTypeByName(
        *m_context, type->get_name());
    assert(llvm_type && 
        "shell LLVM equivelant for struct type has not been created!");

    std::vector<llvm::Type*> fields;
    fields.reserve(type->num_fields());
    for (auto& field : type->fields()) {
        llvm::Type* field_type = translate(field);
        assert(field_type && 
            "could not lower SIIR struct field type to an LLVM equivelant!");
        fields.push_back(field_type);
    }

    llvm_type->setBody(fields);
}

void LLVMTranslatePass::convert(Global* global) {
    llvm::GlobalVariable* GV = translate(global);

    if (global->has_initializer())
        GV->setInitializer(translate(global->get_initializer()));
}

void LLVMTranslatePass::convert(Function* fn) {
    llvm::Function* F = translate(fn);

    //F->addFnAttr(llvm::Attribute::StackProtectStrong);
    //F->addFnAttr("stack-protector-buffer-size", "8");
    
    F->addFnAttr(llvm::Attribute::UWTable);
    F->addFnAttr(llvm::Attribute::NoUnwind);
    F->setUWTableKind(llvm::UWTableKind::Default);

    F->addFnAttr("frame-pointer", "all");
    F->addFnAttr("target-cpu", "x86-64");

    for (auto& arg : fn->args()) {
        llvm::Argument* A = new llvm::Argument(
            translate(arg->get_type()), arg->get_name(), F);
    }

    if (fn->empty())
        return;

    for (auto* curr = fn->front(); curr; curr = curr->next()) {
        llvm::BasicBlock* BB = llvm::BasicBlock::Create(
            *m_context, "bb" + std::to_string(curr->get_number()), F);
        m_blocks.emplace(curr, BB);
    }

    for (auto& [name, local] : fn->locals()) {
        m_builder->SetInsertPoint(&F->front());
        llvm::AllocaInst* alloca = m_builder->CreateAlloca(
            translate(local->get_allocated_type()), nullptr, '_' + name);
        m_locals.emplace(local, alloca);
    }

    for (auto* curr = fn->front(); curr; curr = curr->next()) {
        convert(curr);
    }

    for (auto& [og, phi] : m_delayed_phis) {
        for (auto& incoming : og->get_operand_list()) {
            auto phiop = dynamic_cast<PhiOperand*>(incoming->get_value());
            assert(phiop);

            phi->addIncoming(
                translate(phiop->get_value()), translate(phiop->get_pred()));
        }
    }

    m_delayed_phis.clear();
    m_locals.clear();

    assert(!llvm::verifyFunction(*F, &llvm::outs()));
}

void LLVMTranslatePass::convert(BasicBlock* bb) {
    m_builder->SetInsertPoint(translate(bb));

    for (auto curr = bb->front(); curr; curr = curr->next()) {
        convert(curr);
    }
}

void LLVMTranslatePass::convert(Instruction* inst) {
    switch (inst->opcode()) { 
    case INST_OP_STRING: {
        std::string string = static_cast<ConstantString*>(
            inst->get_operand(0))->get_value();
        
        llvm::GlobalVariable* GV = nullptr;
        auto it = m_strings.find(string);
        if (it != m_strings.end()) {
            GV = it->second;
        } else {
            GV = m_builder->CreateGlobalString(string);
            m_strings.emplace(string, GV);
        }

        m_insts.emplace(inst, GV);
        break;
    }

    case INST_OP_LOAD: {
        llvm::Value* V = m_builder->CreateAlignedLoad(
            translate(inst->get_type()), 
            translate(inst->get_operand(0)), 
            llvm::MaybeAlign(inst->data()));
        m_insts.emplace(inst, V);
        break;
    }

    case INST_OP_STORE: {
        m_builder->CreateAlignedStore(
            translate(inst->get_operand(0)), 
            translate(inst->get_operand(1)), 
            llvm::MaybeAlign(inst->data()));
        break;
    }

    case INST_OP_ACCESS_PTR: {
        llvm::Value* V = m_builder->CreateGEP(
            translate(static_cast<const siir::PointerType*>(inst->get_type())->get_pointee()), 
            translate(inst->get_operand(0)), 
            { translate(inst->get_operand(1)) });
        m_insts.emplace(inst, V);
        break;
    }

    case INST_OP_SELECT: {
        llvm::Value* V = m_builder->CreateSelect(
            translate(inst->get_operand(0)), 
            translate(inst->get_operand(1)), 
            translate(inst->get_operand(2)));
        m_insts.emplace(inst, V);
        break;
    }

    case INST_OP_BRANCH_IF: {
        m_builder->CreateCondBr(
            translate(inst->get_operand(0)), 
            translate(static_cast<BlockAddress*>(inst->get_operand(1))->get_block()), 
            translate(static_cast<BlockAddress*>(inst->get_operand(2))->get_block()));
        break;
    }

    case INST_OP_JUMP: {
        m_builder->CreateBr(
            translate(static_cast<BlockAddress*>(inst->get_operand(0))->get_block()));
        break;
    }

    case INST_OP_PHI: {
        llvm::PHINode* phi = m_builder->CreatePHI(
            translate(inst->get_type()), inst->num_operands());

        m_delayed_phis.push_back({ inst, phi });
        m_insts.emplace(inst, phi);
        break;
    }

    case INST_OP_RETURN: {
        if (inst->num_operands() == 0) {
            m_builder->CreateRetVoid();
        } else {
            m_builder->CreateRet(translate(inst->get_operand(0)));
        }

        break;
    }
    
    case INST_OP_ABORT:
    case INST_OP_UNREACHABLE: {
        m_builder->CreateUnreachable();
        break;
    }
    
    case INST_OP_CALL: {
        if (InlineAsm* iasm = dynamic_cast<InlineAsm*>(inst->get_operand(0))) {
            std::string string = iasm->string();
            std::string constraints = "";

            for (u32 idx = 0, e = iasm->constraints().size(); idx != e; ++idx) {
                std::string constraint = iasm->constraints().at(idx);
                if (constraint.at(0) == '~') {
                    constraints += "~{" + constraint.substr(1) + "}";
                } else {
                    constraints += constraint;
                }
                
                if (idx + 1 != e)
                    constraints += ",";
            }

            const siir::FunctionType* siir_type = 
                dynamic_cast<const siir::FunctionType*>(iasm->get_type());
            assert(siir_type);

            llvm::Type* return_type = nullptr;
            if (siir_type->has_return_type()) {
                return_type = translate(siir_type->get_return_type());
            } else {
                return_type = llvm::Type::getVoidTy(*m_context);
            }

            std::vector<llvm::Type*> param_types(siir_type->num_args(), nullptr);
            for (u32 idx = 0, e = siir_type->num_args(); idx != e; ++idx)
                param_types[idx] = translate(siir_type->args().at(idx));

            llvm::FunctionType* type = llvm::FunctionType::get(
                return_type, param_types, false);

            llvm::InlineAsm* llvm_iasm = llvm::InlineAsm::get(
                type, string, constraints, iasm->has_side_effects());

            std::vector<llvm::Value*> args;
            if (inst->num_operands() > 1)
                args.reserve(inst->num_operands() - 1);

            for (u32 idx = 1, e = inst->num_operands(); idx != e; ++idx) {
                args.push_back(translate(inst->get_operand(idx)));
            }

            llvm::CallInst* call = m_builder->CreateCall(type, llvm_iasm, args);
            for (u32 idx = 0, e = iasm->constraints().size(); idx != e; ++idx) {
                std::string constraint = iasm->constraints().at(idx);
                if (constraint == "=*r" || constraint == "=*m") {
                    call->addParamAttr(idx, llvm::Attribute::get(
                        *m_context, 
                        llvm::Attribute::ElementType, 
                        translate(static_cast<const PointerType*>(siir_type->get_arg(idx))->get_pointee())));
                }
            }

            m_insts.emplace(inst, call);
            break;
        } else {
            llvm::Function* callee = llvm::dyn_cast<llvm::Function>(
                translate(inst->get_operand(0)));
            
            std::vector<llvm::Value*> args;
            if (inst->num_operands() > 1)
                args.reserve(inst->num_operands() - 1);

            for (u32 idx = 1, e = inst->num_operands(); idx != e; ++idx)
                args.push_back(translate(inst->get_operand(idx)));

            llvm::CallInst* call = m_builder->CreateCall(callee, args);
            m_insts.emplace(inst, call);
        }

        break;
    }

    case INST_OP_IADD: {
        llvm::Value* V = m_builder->CreateAdd(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }

    case INST_OP_FADD: {
        llvm::Value* V = m_builder->CreateFAdd(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }

    case INST_OP_ISUB: {
        llvm::Value* V = m_builder->CreateSub(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }

    case INST_OP_FSUB: {
        llvm::Value* V = m_builder->CreateFSub(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_SMUL: 
    case INST_OP_UMUL: {
        llvm::Value* V = m_builder->CreateMul(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_FMUL: {
        llvm::Value* V = m_builder->CreateFMul(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_SDIV: {
        llvm::Value* V = m_builder->CreateSDiv(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_UDIV: {
        llvm::Value* V = m_builder->CreateUDiv(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_FDIV: {
        llvm::Value* V = m_builder->CreateFDiv(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_SREM: {
        llvm::Value* V = m_builder->CreateSRem(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_UREM: {
        llvm::Value* V = m_builder->CreateURem(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_AND: {
        llvm::Value* V = m_builder->CreateAnd(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_OR: {
        llvm::Value* V = m_builder->CreateOr(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_XOR: {
        llvm::Value* V = m_builder->CreateXor(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_SHL: {
        llvm::Value* V = m_builder->CreateShl(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_SHR: {
        llvm::Value* V = m_builder->CreateLShr(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_SAR: {
        llvm::Value* V = m_builder->CreateAShr(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_NOT: {
        llvm::Value* V = m_builder->CreateNot(translate(inst->get_operand(0)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_INEG: {
        llvm::Value* V = m_builder->CreateNeg(translate(inst->get_operand(0)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_FNEG: {
        llvm::Value* V = m_builder->CreateFNeg(translate(inst->get_operand(0)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_SEXT: {
        llvm::Value* V = m_builder->CreateSExt(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_ZEXT: {
        llvm::Value* V = m_builder->CreateZExt(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_FEXT: {
        llvm::Value* V = m_builder->CreateFPExt(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_ITRUNC: {
        llvm::Value* V = m_builder->CreateTrunc(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_FTRUNC: {
        llvm::Value* V = m_builder->CreateFPTrunc(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_SI2FP: {
        llvm::Value* V = m_builder->CreateSIToFP(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_UI2FP: {
        llvm::Value* V = m_builder->CreateUIToFP(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_FP2SI: {
        llvm::Value* V = m_builder->CreateFPToSI(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_FP2UI: {
        llvm::Value* V = m_builder->CreateFPToUI(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_P2I: {
        llvm::Value* V = m_builder->CreatePtrToInt(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_I2P: {
        llvm::Value* V = m_builder->CreateIntToPtr(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_REINTERPET: {
        llvm::Value* V = m_builder->CreatePointerCast(
            translate(inst->get_operand(0)), translate(inst->get_type()));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_IEQ: {
        llvm::Value* V = m_builder->CreateICmpEQ(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_INE: {
        llvm::Value* V = m_builder->CreateICmpNE(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_OEQ: {
        llvm::Value* V = m_builder->CreateFCmpOEQ(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_ONE: {
        llvm::Value* V = m_builder->CreateFCmpONE(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_UNEQ: {
        llvm::Value* V = m_builder->CreateFCmpUEQ(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_UNNE: {
        llvm::Value* V = m_builder->CreateFCmpUNE(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_SLT: {
        llvm::Value* V = m_builder->CreateICmpSLT(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_SLE: {
        llvm::Value* V = m_builder->CreateICmpSLE(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_SGT: {
        llvm::Value* V = m_builder->CreateICmpSGT(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_SGE: {
        llvm::Value* V = m_builder->CreateICmpSGE(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_ULT: {
        llvm::Value* V = m_builder->CreateICmpULT(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_ULE: {
        llvm::Value* V = m_builder->CreateICmpULE(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_UGT: {
        llvm::Value* V = m_builder->CreateICmpUGT(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_UGE: {
        llvm::Value* V = m_builder->CreateICmpUGE(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_OLT: {
        llvm::Value* V = m_builder->CreateFCmpOLT(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_OLE: {
        llvm::Value* V = m_builder->CreateFCmpOLE(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_OGT: {
        llvm::Value* V = m_builder->CreateFCmpOGT(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_OGE: {
        llvm::Value* V = m_builder->CreateFCmpOGE(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_UNLT: {
        llvm::Value* V = m_builder->CreateFCmpULT(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_UNLE: {
        llvm::Value* V = m_builder->CreateFCmpULE(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_UNGT: {
        llvm::Value* V = m_builder->CreateFCmpUGT(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    case INST_OP_CMP_UNGE: {
        llvm::Value* V = m_builder->CreateFCmpUGE(
            translate(inst->get_operand(0)), translate(inst->get_operand(1)));
        m_insts.emplace(inst, V);
        break;
    }
    
    default:
        assert(false && "no LLVM equivelant for SIIR instruction!");
    }
}

#endif // STMC_LLVM_SUPPORT
