//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/analysis/AMD64LoweringPass.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Function.hpp"
#include "lir/graph/Global.hpp"
#include "lir/graph/Instruction.hpp"
#include "lir/graph/Type.hpp"
#include "lir/machine/AMD64.hpp"
#include "lir/machine/FunctionABI.hpp"
#include "lir/machine/MachineConstant.hpp"
#include "lir/machine/MachineFunction.hpp"
#include "lir/machine/MachineOp.hpp"
#include "lir/machine/MachineOperand.hpp"
#include "lir/machine/MachineRegister.hpp"
#include "lir/machine/Register.hpp"

#include <cstdint>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

using namespace lir;

/// Convert the given comparison |predicate| to an equivelant conditional jump.
static AMD64_Op cmp_to_jcc(Cmp::Predicate predicate) {
    switch (predicate) {
        case Cmp::Predicate::IEq:
        case Cmp::Predicate::FEq:
            return AMD64_JE;
        case Cmp::Predicate::INe:
        case Cmp::Predicate::FNe:
            return AMD64_JNE;
        case Cmp::Predicate::Slt:
            return AMD64_JL;
        case Cmp::Predicate::Ult:
        case Cmp::Predicate::Flt:
            return AMD64_JB;
        case Cmp::Predicate::Sle:
            return AMD64_JLE;
        case Cmp::Predicate::Ule:
        case Cmp::Predicate::Fle:
            return AMD64_JBE;
        case Cmp::Predicate::Sgt:
            return AMD64_JG;
        case Cmp::Predicate::Ugt:
        case Cmp::Predicate::Fgt:
            return AMD64_JA;
        case Cmp::Predicate::Sge:
            return AMD64_JGE;
        case Cmp::Predicate::Uge:
        case Cmp::Predicate::Fge:
            return AMD64_JAE;
    }
}

/// Convert the given comparison |predicate| to an equivelant conditional set.
static AMD64_Op cmp_to_setcc(Cmp::Predicate predicate) {
    switch (predicate) {
        case Cmp::Predicate::IEq:
        case Cmp::Predicate::FEq:
            return AMD64_SETE;
        case Cmp::Predicate::INe:
        case Cmp::Predicate::FNe:
            return AMD64_SETNE;
        case Cmp::Predicate::Slt:
            return AMD64_SETL;
        case Cmp::Predicate::Ult:
        case Cmp::Predicate::Flt:
            return AMD64_SETB;
        case Cmp::Predicate::Sle:
            return AMD64_SETLE;
        case Cmp::Predicate::Ule:
        case Cmp::Predicate::Fle:
            return AMD64_SETBE;
        case Cmp::Predicate::Sgt:
            return AMD64_SETG;
        case Cmp::Predicate::Ugt:
        case Cmp::Predicate::Fgt:
            return AMD64_SETA;
        case Cmp::Predicate::Sge:
            return AMD64_SETGE;
        case Cmp::Predicate::Uge:
        case Cmp::Predicate::Fge:
            return AMD64_SETAE;
    }
}

/// Flip the given conditional jump mnemonic about the predicate.
static AMD64_Op flip_jcc(AMD64_Op Jcc) {
    switch (Jcc) {
        case AMD64_JE:
        case AMD64_JNE:
        case AMD64_JZ:
        case AMD64_JNZ:
            return Jcc;
        case AMD64_JL:
            return AMD64_JG;
        case AMD64_JLE:
            return AMD64_JGE;
        case AMD64_JG:
            return AMD64_JL;
        case AMD64_JGE:
            return AMD64_JLE;
        case AMD64_JA:
            return AMD64_JB;
        case AMD64_JAE:
            return AMD64_JBE;
        case AMD64_JB:
            return AMD64_JA;
        case AMD64_JBE:
            return AMD64_JAE;
        default:
            assert(false && "invalid Jcc op!");
    }
}

/// Flip the given conditional set mnemonic about the predicate.
static AMD64_Op flip_setcc(AMD64_Op SETcc) {
    switch (SETcc) {
        case AMD64_SETE:
        case AMD64_SETNE:
        case AMD64_SETZ:
        case AMD64_SETNZ:
            return SETcc;
        case AMD64_SETL:
            return AMD64_SETG;
        case AMD64_SETLE:
            return AMD64_SETGE;
        case AMD64_SETG:
            return AMD64_SETL;
        case AMD64_SETGE:
            return AMD64_SETLE;
        case AMD64_SETA:
            return AMD64_SETB;
        case AMD64_SETAE:
            return AMD64_SETBE;
        case AMD64_SETB:
            return AMD64_SETA;
        case AMD64_SETBE:
            return AMD64_SETAE;
        default:
            assert(false && "invalid SETcc op!");
    }
}

/// Stringify the given |inst| by printing its definition as a reusable string.
static std::string stringify_inst(const Instruction *inst) {
    std::stringstream ss;
    inst->print(ss, PrintPolicy::Def);
    return ss.str();
}

//>==---------------------------------------------------------------------------
//                          AMD64LoweringPass Implementation
//>==---------------------------------------------------------------------------

void AMD64LoweringPass::run() {
    for (const Global* global : m_cfg.get_globals()) {
        std::vector<MachineConstant> bytes = {};
        
        if (global->has_initializer()) {
            lower_constant(global->get_initializer(), bytes);
        } else {
            auto ptr = dynamic_cast<PointerType*>(global->get_type());
            assert(ptr);

            bytes.push_back(MachineConstant(m_obj.get_machine().get_type_size(ptr->get_pointee()) / 8));
        }

        MachineData* MD = new MachineData(
            global->get_name(), 
            bytes,
            global->has_linkage(Global::LinkageType::Public), 
            !global->is_mutable());
        assert(MD && "failed to create new machine data!");

        m_obj.get_globals().emplace(global->get_name(), MD);
    }

    // Lower each function and it's blocks to machine functions & labels.
    for (const Function* func : m_cfg.get_functions()) {
        // Empty functions should not be lowered, they should either be resolved at link time or 
        // with some library.
        //if (func->empty())
       //     continue;

        FunctionABI abi = FunctionABI(m_mach, func);

        MachineFunction* MF = new MachineFunction(
            &m_obj, 
            abi, 
            func->get_name(), 
            func->has_linkage(Function::LinkageType::Public));
        assert(MF && "failed to create new machine function!");

        const BasicBlock* curr = func->get_head();
        while (curr) {
            MachineLabel* ML = new MachineLabel(MF);
            assert(ML && "failed to create new machine label!");

            curr = curr->get_next();
        }
    }

    // Properly lower the instruction list of each function, on a block basis.
    for (const Function *func : m_cfg.get_functions()) {
        if (func->empty())
            continue;

        MachineFunction *MF = m_obj.get_function(func->get_name());
        assert(MF);
        assert(MF->num_labels() == func->size());

        m_func = MF;
        construct_stack_frame(func);

        uint32_t pos = 0;
        for (auto block = func->get_head(); block; block = block->get_next()) {
            MachineLabel *ML = MF->get_label(pos);
            assert(ML);
            
            m_insert = ML;

            if (pos == 0) {
                emit(static_cast<uint32_t>(Intrinsic::Stack_Setup));
                emit(static_cast<uint32_t>(Intrinsic::Stack_Reserve));
            }

            const Instruction *curr = block->get_head();
            while (curr) {
                lower_inst(curr);
                curr = curr->get_next();
            }
            
            pos++;
        }

        MF->update_positions();
    }
}

uint8_t AMD64LoweringPass::get_subreg_byte(const Type *type) const {
    assert(type && "type cannot be null!");
    assert(m_mach.is_scalar(type) && "type must be scalar!");

    switch (m_mach.get_type_size(type)) {
        case 8:
            return 1;
        case 16:
            return 2;
        case 32:
            return 4;
        case 64:
            return 8;
        default:
            assert(false && "invalid scalar type size!");
    }
}

AMD64_Op AMD64LoweringPass::get_sized_op(const Type* type, const std::array<AMD64_Op, 4>& gp) {
    if (type->is_integer_type() || type->is_pointer_type()) {
        const std::unordered_map<uint32_t, AMD64_Op> table = {
            { 8, gp[0] }, { 16, gp[1] }, { 32, gp[2] }, { 64, gp[3] }
        };

        return table.at(m_mach.get_type_size(type));
    } else {
        assert(false && "(1) non-integer op!");
    }
}

AMD64_Op AMD64LoweringPass::getSizedOp(const Type* type, const std::array<AMD64_Op, 2>& fp) {
    if (type->is_float_type(32)) {
        return fp[0];
    } else if (type->is_float_type(64)) {
        return fp[1];
    } else {
        assert(false && "(3) non-fp op!");
    }
}

AMD64_Op AMD64LoweringPass::get_sized_op(const Type *type, 
                                         const std::array<AMD64_Op, 4> &gp, 
                                         const std::array<AMD64_Op, 2> &fp) {
    if (type->is_integer_type() || type->is_pointer_type()) {
        return get_sized_op(type, gp);
    } else if (type->is_float_type()) {
        const std::unordered_map<uint32_t, AMD64_Op> table = {
            { 32, fp[0] }, { 64, fp[1] }
        };

        return table.at(m_mach.get_type_size(type));
    } else {
        assert(false && "(2) non-scalar op!");
    }
}

AMD64_Op AMD64LoweringPass::get_move_op(const Type *type) {
    return get_sized_op(
        type, 
        { AMD64_MOV8, AMD64_MOV16, AMD64_MOV32, AMD64_MOV64 }, 
        { AMD64_MOVSS, AMD64_MOVSD }
    );
}

AMD64_Op AMD64LoweringPass::get_cmp_op(const Type *type) {
    return get_sized_op(
        type,
        { AMD64_CMP8, AMD64_CMP16, AMD64_CMP32, AMD64_CMP64 },
        { AMD64_UCOMISS, AMD64_UCOMISD }
    );
}

AMD64_Op AMD64LoweringPass::getOpIAdd(const Type* type) {
    return get_sized_op(type, { AMD64_ADD8, AMD64_ADD16, AMD64_ADD32, AMD64_ADD64 });
}

AMD64_Op AMD64LoweringPass::getOpISub(const Type* type) {
    return get_sized_op(type, { AMD64_SUB8, AMD64_SUB16, AMD64_SUB32, AMD64_SUB64 });
}

AMD64_Op AMD64LoweringPass::getOpIMul(const Type* type) {
    return get_sized_op(type, { AMD64_IMUL8, AMD64_IMUL16, AMD64_IMUL32, AMD64_IMUL64 });
}

AMD64_Op AMD64LoweringPass::getOpSDiv(const Type* type) {
    return get_sized_op(type, { AMD64_IDIV8, AMD64_IDIV16, AMD64_IDIV32, AMD64_IDIV64 });
}

AMD64_Op AMD64LoweringPass::getOpUDiv(const Type* type) {
    return get_sized_op(type, { AMD64_DIV8, AMD64_DIV16, AMD64_DIV32, AMD64_DIV64 });
}

AMD64_Op AMD64LoweringPass::getOpAnd(const Type* type) {
    return get_sized_op(type, { AMD64_AND8, AMD64_AND16, AMD64_AND32, AMD64_AND64 });
}

AMD64_Op AMD64LoweringPass::getOpOr(const Type* type) {
    return get_sized_op(type, { AMD64_OR8, AMD64_OR16, AMD64_OR32, AMD64_OR64 });
}

AMD64_Op AMD64LoweringPass::getOpXor(const Type* type) {
    return get_sized_op(type, { AMD64_XOR8, AMD64_XOR16, AMD64_XOR32, AMD64_XOR64 });
}

AMD64_Op AMD64LoweringPass::getOpShl(const Type* type) {
    return get_sized_op(type, { AMD64_SHL8, AMD64_SHL16, AMD64_SHL32, AMD64_SHL64 });
}

AMD64_Op AMD64LoweringPass::getOpShr(const Type* type) {
    return get_sized_op(type, { AMD64_SHR8, AMD64_SHR16, AMD64_SHR32, AMD64_SHR64 });
}

AMD64_Op AMD64LoweringPass::getOpSar(const Type* type) {
    return get_sized_op(type, { AMD64_SAR8, AMD64_SAR16, AMD64_SAR32, AMD64_SAR64 });
}

AMD64_Op AMD64LoweringPass::getOpFAdd(const Type* type) {
    return getSizedOp(type, { AMD64_ADDSS, AMD64_ADDSD });
}

AMD64_Op AMD64LoweringPass::getOpFSub(const Type* type) {
    return getSizedOp(type, { AMD64_SUBSS, AMD64_SUBSD });
}

AMD64_Op AMD64LoweringPass::getOpFMul(const Type* type) {
    return getSizedOp(type, { AMD64_MULSS, AMD64_MULSD });
}

AMD64_Op AMD64LoweringPass::getOpFDiv(const Type* type) {
    return getSizedOp(type, { AMD64_DIVSS, AMD64_DIVSD });
}

AMD64_Op AMD64LoweringPass::getOpNot(const Type* type) {
    return get_sized_op(type, { AMD64_NOT8, AMD64_NOT16, AMD64_NOT32, AMD64_NOT64 });
}

AMD64_Op AMD64LoweringPass::getOpINeg(const Type* type) {
    return get_sized_op(type, { AMD64_NEG8, AMD64_NEG16, AMD64_NEG32, AMD64_NEG64 });
}

AMD64_Op AMD64LoweringPass::getOpSS2SI(const Type* type) {
    return get_sized_op(type, {
        AMD64_CVTTSS2SI8, AMD64_CVTTSS2SI16, AMD64_CVTTSS2SI32, AMD64_CVTTSS2SI64,  
    });
}

AMD64_Op AMD64LoweringPass::getOpSD2SI(const Type* type) {
    return get_sized_op(type, {
        AMD64_CVTTSD2SI8, AMD64_CVTTSD2SI16, AMD64_CVTTSD2SI32, AMD64_CVTTSD2SI64,  
    });
}

AMD64_Op AMD64LoweringPass::getOpSS2UI(const Type* type) {
    return get_sized_op(type, {
        AMD64_VCVCTSS2USI8, AMD64_VCVCTSS2USI16, AMD64_VCVCTSS2USI32, AMD64_VCVCTSS2USI64, 
    });
}

AMD64_Op AMD64LoweringPass::getOpSD2UI(const Type* type) {
    return get_sized_op(type, {
        AMD64_VCVCTSD2USI8, AMD64_VCVCTSD2USI16, AMD64_VCVCTSD2USI32, AMD64_VCVCTSD2USI64, 
    });
}

Register AMD64LoweringPass::create_vreg(RegisterClass cls) {
    static uint32_t id = Register::VIRTUAL_BARRIER + 1;

    Register vreg(id++, cls);
    return vreg;
}

Register AMD64LoweringPass::get_vreg_from_def(const Instruction *inst) {
    assert(inst && "inst cannot be null!");
    assert(inst->is_def() && "inst is not a def!");

    auto it = m_defs.find(inst->def());
    if (it != m_defs.end())
        return it->second;

    auto cls = RegisterClass::GeneralPurpose;
    if (inst->get_type()->is_float_type())
        cls = RegisterClass::FloatingPoint;

    Register vreg = create_vreg(cls);
    m_defs.emplace(inst->def(), vreg);
    return vreg;
}

MachineOperand AMD64LoweringPass::to_operand(const Value *value) {
    if (auto integer = dynamic_cast<const Integer*>(value)) {
        return MachineOperand(integer->get_value());
    } else if (auto null = dynamic_cast<const Null*>(value)) {
        return MachineOperand(0l);
    } else if (auto inst = dynamic_cast<const Instruction*>(value)) {
        Register vreg = get_vreg_from_def(inst);
        uint8_t subreg = get_subreg_byte(value->get_type());

        return MachineOperand(MachineRegister { vreg, subreg });
    } else if (auto param = dynamic_cast<const Parameter*>(value)) {
        const FunctionABI &abi = m_func->abi();
        const uint32_t index = param->get_index();

        const FunctionABI::Location &loc = abi.getParamLocation(index);
        MachineRegister BP(RBP, 8);

        return MachineOperand(Memory { BP, loc.offset + 16 });
    } else if (auto func = dynamic_cast<const Function*>(value)) {
        MachineFunction *MF = m_obj.get_function(func->get_name());
        assert(MF && "function not lowered!");

        return MachineOperand(MF);
    } else if (auto global = dynamic_cast<const Global*>(value)) {
        MachineData *MD = m_obj.get_global(global->get_name());
        assert(MD && "global not lowered!");

        return MachineOperand(MD);
    } else if (auto local = dynamic_cast<const Local*>(value)) {
        auto it = m_locals.find(local);
        assert(it != m_locals.end() && "local not lowered!");

        MachineLocal *ML = it->second;
        assert(ML);

        return MachineOperand(ML);
    }

    assert(false && "invalid value!");
}

MachineOp &AMD64LoweringPass::emit(uint32_t op, 
                                   const MachineOp::Operands &operands) {
    assert(m_insert && "no insertion point set!");

    MachineOp *inst = new MachineOp(op, operands, m_insert);
    return *inst;
}

void AMD64LoweringPass::construct_stack_frame(const Function *func) {
    m_locals.clear();

    StackFrame &frame = m_func->get_stack_frame();
    int32_t offset = 0;

    for (const auto &[name, local] : func->get_locals()) {
        const Type *alloc = local->get_allocated_type();
        const uint32_t size = m_mach.get_type_size(alloc) / 8;
        const uint32_t align = m_mach.get_type_align(alloc) / 8;

        MachineLocal *ML = new MachineLocal(&frame, offset, size, align);
        assert(ML);

        m_locals.emplace(local, ML);
        offset += size;
    }
}

void AMD64LoweringPass::lower_inst(const Instruction *inst) {
    assert(inst && "inst cannot be null!");

    if (auto C = dynamic_cast<const Const*>(inst)) {
        lower_const(C);
    } else if (auto L = dynamic_cast<const Load*>(inst)) {
        lower_load(L);
    } else if (auto S = dynamic_cast<const Store*>(inst)) {
        lower_store(S);
    } else if (auto A = dynamic_cast<const Access*>(inst)) {
        lower_access(A);
    } else if (auto O = dynamic_cast<const Offptr*>(inst)) {
        lower_offptr(O);
    } else if (auto C = dynamic_cast<const Call*>(inst)) {
        lower_call(C);
    } else if (auto R = dynamic_cast<const Ret*>(inst)) {
        lower_ret(R);
    } else if (auto J = dynamic_cast<const Jump*>(inst)) {
        lower_jump(J);
    } else if (auto B = dynamic_cast<const Brif*>(inst)) {
        lower_brif(B);
    } else if (auto P = dynamic_cast<const Phi*>(inst)) {
        lower_phi(P);
    } else if (auto U = dynamic_cast<const Unop*>(inst)) {
        lower_unop(U);
    } else if (auto B = dynamic_cast<const Binop*>(inst)) {
        lower_binop(B);
    } else if (auto C = dynamic_cast<const Cast*>(inst)) {
        lower_cast(C);
    } else if (auto C = dynamic_cast<const Cmp*>(inst)) {
        lower_cmp(C);
    }
}

void AMD64LoweringPass::lower_const(const Const *C) {
    const Constant *constant = C->get_value();
    assert(constant);

    MachineRegister dReg = MachineRegister()
        .setReg(get_vreg_from_def(C))
        .setSubreg(get_subreg_byte(C->get_type()));

    if (auto integer = dynamic_cast<const Integer*>(constant)) {
        // MOVABS lets us put a 64-bit integer into a register.
        emit(AMD64_MOVABS)
            .add_imm(integer->get_value())
            .add_reg(dReg)
            .add_comment(stringify_inst(C));
    } else if (auto fp = dynamic_cast<const Float*>(constant)) {
        // Floats need to be pooled within the local function.
        std::vector<MachineConstant> bytes = {};
        lower_constant(fp, bytes);

        // Materialie a new floating point constant from the function pool.
        ConstantPool &pool = m_func->get_pool();
        MachineData *MD = pool.materialize(bytes);

        emit(get_move_op(fp->get_type()))
            .add_data(MD)
            .add_reg(dReg)
            .add_comment(stringify_inst(C));
    } else if (auto string = dynamic_cast<const String*>(constant)) {
        // Strings must also be pooled into the local function.
        std::vector<MachineConstant> bytes = {};
        lower_constant(string, bytes);

        ConstantPool &pool = m_func->get_pool();
        MachineData *MD = pool.materialize(bytes);

        emit(AMD64_LEA64)
            .add_data(MD)
            .add_reg(dReg)
            .add_comment(stringify_inst(C));
    } else if (auto aggregate = dynamic_cast<const Aggregate*>(constant)) {
        assert(false && "(2) non-scalar!");
    } else {
        assert(false && "invalid constant!");
    }
}

void AMD64LoweringPass::lower_load(const Load *L) {
    MachineOperand source = to_operand(L->get_addr());

    if (source.is_reg()) {
        // Source is some defined value, must be reinterpreted as a memory
        // access at an offset of 0.
        source = MachineOperand(Memory { source.reg(), 0 });
    }

    MachineRegister dest(get_vreg_from_def(L), get_subreg_byte(L->get_type()));

    emit(get_move_op(L->get_type()), { source })
        .add_reg(dest)
        .add_comment(stringify_inst(L));
}

void AMD64LoweringPass::lower_store(const Store *S) {
    MachineOperand dest = to_operand(S->get_addr());

    if (dest.is_reg()) {
        // Destination is some defined value, must be reinterpreted as a memory
        // access at an offset of 0.
        dest = MachineOperand(Memory { dest.reg(), 0 });
    }

    const Value *value = S->get_value();
    const MachineOperand source = to_operand(value);

    if (dynamic_cast<const Parameter*>(value)) {
        // Function arguments arrive via the stack, and since the destination of a store is a 
        // memory reference, the argument must first be moved to a temporary.
        RegisterClass cls = RegisterClass::GeneralPurpose;
        if (value->get_type()->is_float_type())
            cls = RegisterClass::FloatingPoint;
        
        MachineRegister tmp = create_vreg(cls);
        tmp.setSubreg(get_subreg_byte(value->get_type()));

        emit(get_move_op(value->get_type()), { source, tmp })
            .add_comment(stringify_inst(S));

        emit(get_move_op(value->get_type()), { tmp, dest });
    } else {
        emit(get_move_op(value->get_type()), { source, dest })
            .add_comment(stringify_inst(S));
    }
}

void AMD64LoweringPass::lower_access(const Access* A) {
    const MachineOperand index = to_operand(A->get_index());
    assert(index.is_imm() && "Access index is not an immediate!");;

    auto ptr = dynamic_cast<const PointerType*>(A->get_base()->get_type());
    assert(ptr && "Access base is not a pointer!");

    auto structure = dynamic_cast<const StructType*>(ptr->get_pointee());
    assert(structure && "Access base is not a structure!");

    const MachineOperand source = to_operand(A->get_base());

    // @Todo: Assumes immediate index.
    uint32_t offset = m_mach.get_field_offset(structure, index.imm());

    MachineRegister MR(get_vreg_from_def(A), get_subreg_byte(A->get_type()));

    if (dynamic_cast<const Local*>(A->get_base()) || dynamic_cast<const Global*>(A->get_base())) {
        emit(AMD64_LEA64, { source })
            .add_reg(MR)
            .add_comment(stringify_inst(A));
    } else {
        emit(AMD64_MOV64, { source })
            .add_reg(MR)
            .add_comment(stringify_inst(A));
    }

    if (offset != 0) {
        // If the field offset is non-zero, then add it to the base pointer.
        emit(AMD64_ADD64)
            .add_imm(offset)
            .add_reg(MR);
    }
}

void AMD64LoweringPass::lower_offptr(const Offptr *O) {
    const MachineOperand source = to_operand(O->get_base());
    const MachineOperand index = to_operand(O->get_index());

    const MachineRegister DR(get_vreg_from_def(O), 8);

    emit(get_move_op(O->get_type()), { source })
        .add_reg(DR)
        .add_comment(stringify_inst(O));

    auto underlying = dynamic_cast<const PointerType*>(O->get_base()->get_type());
    assert(underlying && "Offptr base is not a pointer!");

    uint32_t bytes = m_mach.get_type_size(underlying->get_pointee()) / 8;

    if (index.is_imm()) {
        // Index is immediate, so we can multiply it at compile-time by the size of the underlying.
        int64_t offset = static_cast<int64_t>(bytes) * index.imm();

        emit(AMD64_ADD64, { offset })
            .add_reg(DR);
    } else {
        // Index is dynamic, so we have to multiply it at runtime by the size of the underlying.
        emit(AMD64_IMUL64, { bytes, index });
        emit(AMD64_ADD64, { index })
            .add_reg(DR);
    }
}

void AMD64LoweringPass::lower_call(const Call* C) {
    emit(static_cast<uint32_t>(Intrinsic::Callsite_Set))
        .add_comment(stringify_inst(C));

    // @Todo: change with function pointers/callbacks.
    // 
    // Obviously direct calls can be treated specially, but this is done for the sake of the ABI.
    // Speaking of, it should be agnostic of function, and tie to function type.
    auto callee = dynamic_cast<const Function*>(C->get_callee());
    assert(callee && "Call callee is not a function!");

    MachineFunction* MF = m_obj.get_function(callee->get_name());
    assert(MF && "Call callee does not exist!");

    const FunctionABI& abi = MF->abi();

    for (uint32_t i = 0; i < C->num_args(); ++i) {
        const FunctionABI::Location& loc = abi.getParamLocation(i);
        const Value* arg_val = C->get_arg(i);
        const MachineOperand arg_op = to_operand(arg_val);
        
        if (dynamic_cast<const Local*>(arg_val) || dynamic_cast<const Global*>(arg_val)) {
            // Local/global operands are pointers coming from memory sections (stack & data, 
            // respectively), and must be addressed using LEA.
            // 
            // Since the argument destination is on the stack, and both operands cannot be memory 
            // references, the address needs to be moved to a temporary.

            MachineRegister tmp = create_vreg(RegisterClass::GeneralPurpose);

            emit(AMD64_LEA64, { arg_op, tmp });
            emit(AMD64_MOV64, { tmp })
                .add_mem(MachineRegister(RSP, 8), loc.offset);
        } else if (dynamic_cast<const Parameter*>(arg_val)) {
            // Parameter operands come from the stack, and must be moved to a temporary first,
            // since the destination is also on the stack (a reference to memory).

            RegisterClass cls = RegisterClass::GeneralPurpose;
            if (arg_val->get_type()->is_float_type())
                cls = RegisterClass::FloatingPoint;

            MachineRegister tmp = create_vreg(cls);

            emit(get_move_op(arg_val->get_type()), { arg_op, tmp });
            emit(get_move_op(arg_val->get_type()), { tmp })
                .add_mem(MachineRegister(RSP, 8), loc.offset);
        } else {
            emit(get_move_op(arg_val->get_type()), { arg_op })
                .add_mem(MachineRegister(RSP, 8), loc.offset);
        }
    }

    if (abi.hasResult()) {
        // The result register needs to implicitly defined by the call.
        MachineRegister rReg = MachineRegister()
            .setReg(abi.getResultLocation().reg)
            .setIsDef()
            .setIsImplicit();

        emit(AMD64_CALL64, { MF,  rReg });

        const MachineRegister dest_op = get_vreg_from_def(C);

        rReg.setIsUse();
        rReg.setIsExplicit();
        rReg.setIsExpired();

        emit(get_move_op(C->get_type()), { rReg, dest_op });
    } else {
        emit(AMD64_CALL64, { MF });
    }

    emit(static_cast<uint32_t>(Intrinsic::Callsite_End));
}

void AMD64LoweringPass::lower_ret(const Ret* R) {
    if (R->has_value()) {
        // Resolve the ABI for this function and check that it expects a result.
        const FunctionABI& abi = m_func->abi();
        assert(abi.hasResult() && "Ret has a value, but ABI does not expect one!");

        // Get the location to move the return value to.
        const FunctionABI::Location& loc = abi.getResultLocation();
        const Value* value = R->get_value();
        const MachineOperand result = to_operand(value);

        MachineRegister rReg = MachineRegister()
            .setReg(loc.reg)
            .setSubreg(get_subreg_byte(value->get_type()))
            .setIsDef()
            .setIsExplicit();

        AMD64_Op op = get_move_op(value->get_type());

        // Move the return value to the location specified by the ABI.
        emit(get_move_op(value->get_type()), { result })
            .add_reg(rReg)
            .add_comment(stringify_inst(R));

        // (1) Restore the function stack frame with this return.
        emit(static_cast<uint32_t>(Intrinsic::Stack_Restore));

        rReg.setIsUse();
        rReg.setIsImplicit();
        rReg.setIsExpired();

        emit(AMD64_RET64)
            .add_reg(rReg);
    } else {
        // See (1).
        emit(static_cast<uint32_t>(Intrinsic::Stack_Restore))
            .add_comment(stringify_inst(R));

        emit(AMD64_RET64);
    }
}

void AMD64LoweringPass::lower_jump(const Jump* J) {
    // Simply jump directly to the sole destination label.
    emit(AMD64_JMP)
        .add_label(m_func->get_label(J->get_dest()->position()))
        .add_comment(stringify_inst(J));
}

void AMD64LoweringPass::lower_brif(const Brif* B) {
    MachineOperand cond = to_operand(B->get_cond());

    if (cond.is_reg()) {
        // If the condition is coming from a register, make sure it's using 
        // only the lowest byte.
        cond.reg().setSubreg(1);
    }

    // Emit a zero comparison on the Brif condition.
    emit(AMD64_CMP8)
        .add_imm(0)
        .add_operand(cond)
        .add_comment(stringify_inst(B));

    // If the condition != 0 i.e. "true", then jump to the true label.
    emit(AMD64_JNE)
        .add_label(m_func->get_label(B->get_true_dest()->position()));

    // Otherwise, jump to the false label.
    emit(AMD64_JMP)
        .add_label(m_func->get_label(B->get_false_dest()->position()));
}

void AMD64LoweringPass::lower_phi(const Phi* P) {
    const Type* type = P->get_type();

    MachineRegister dReg = MachineRegister()
        .setReg(get_vreg_from_def(P))
        .setSubreg(get_subreg_byte(type));

    MachineLabel* curr = m_insert;

    for (uint32_t i = 0; i < P->num_edges(); ++i) {
        Phi::Edge edge = P->get_edge(i);

        m_insert = m_func->get_label(edge.pred->position());
        assert(m_insert);

        MachineOperand source = to_operand(edge.value);

        MachineOp move = MachineOp(get_move_op(type), { source, dReg })
            .add_comment(stringify_inst(P));
    }

    m_insert = curr;
}

void AMD64LoweringPass::lower_unop(const Unop* U) {
    const Value* value = U->get_value();
    
    switch (U->op()) 
    {
        case Unop::Op::Not: {
            const MachineOperand source = to_operand(value);

            MachineRegister dReg = get_vreg_from_def(U);

            emit(get_move_op(value->get_type()), { source, dReg })
                .add_comment(stringify_inst(U));

            emit(getOpNot(value->get_type()), { dReg });
            break;
        }
        case Unop::Op::INeg: {
            const MachineOperand source = to_operand(value);

            MachineRegister dReg = get_vreg_from_def(U);

            emit(get_move_op(value->get_type()), { source, dReg })
                .add_comment(stringify_inst(U));

            emit(getOpINeg(value->get_type()), { dReg });
            break;
        }
        case Unop::Op::FNeg: {
            assert(false && "FNeg not implemented!");
            break;
        }
    }
}

void AMD64LoweringPass::lower_binop(const Binop* B) {
    const Type* type = B->get_type();

    MachineOperand lhs = to_operand(B->get_lhs());
    MachineOperand rhs = to_operand(B->get_rhs());

    MachineRegister dReg = MachineRegister()
        .setReg(get_vreg_from_def(B))
        .setSubreg(get_subreg_byte(type));

    switch (B->op()) 
    {
        case Binop::Op::IAdd: {
            if (rhs.is_imm()) {
                const MachineOperand tmp = lhs;
                lhs = rhs;
                rhs = tmp;
            }

            emit(getOpIAdd(type), { lhs, rhs })
                .add_comment(stringify_inst(B));

            emit(get_move_op(type), { rhs, dReg });

            break;
        }

        case Binop::Op::ISub: {
            if (lhs.is_imm()) {
                emit(get_move_op(type), { lhs, dReg })
                    .add_comment(stringify_inst(B));

                emit(getOpISub(type), { rhs, dReg });
            } else {
                emit(getOpISub(type), { rhs, lhs })
                    .add_comment(stringify_inst(B));

                emit(get_move_op(type), { lhs, dReg });
            }
            
            break;
        }

        case Binop::Op::IMul: {
            if (rhs.is_imm()) {
                const MachineOperand tmp = lhs;
                lhs = rhs;
                rhs = tmp;
            }

            emit(get_move_op(type), { lhs, dReg })
                .add_comment(stringify_inst(B));

            emit(getOpIMul(type), { rhs, dReg });
            
            break;
        }

        case Binop::Op::SDiv:
        case Binop::Op::UDiv:
        case Binop::Op::SMod:
        case Binop::Op::UMod: {
            emit(get_move_op(type), { lhs })
                // RAX is explicitly defined by the move.
                .add_reg({ RAX, get_subreg_byte(type), true })
                .add_comment(stringify_inst(B));

            emit(get_move_op(type), { rhs, dReg });

            const bool is_mod = (B->op() == Binop::Op::SMod) || (B->op() == Binop::Op::UMod);

            if (B->op() == Binop::Op::SDiv || B->op() == Binop::Op::SMod) {
                // IDIV: RAX = RDX:RAX / r/m, RDX = remainder

                emit(AMD64_CQO)
                    // CQO implicitly reads the value of RAX.
                    .add_reg({ RAX, 8, false, true })
                    // RDX is implicitly written to, but whose value is used by the division op.
                    .add_reg({ RDX, 8, true, true });

                emit(getOpSDiv(type), { dReg })
                    // RAX is implicitly used, and its value expires.
                    .add_reg({ RAX, 8, false, true, true })
                    // RDX is implicitly used, and its value expires.
                    .add_reg({ RDX, 8, false, true, true })
                    // If this is a mod, the remainder is used, and RAX (quotient) expires.
                    .add_reg({ RAX, 8, true, true, is_mod })
                    // If this is a div, the quotient is uised, and RDX (the remainder) expires.
                    .add_reg({ RDX, 8, true, true, !is_mod });
            } else {
                // DIV: RAX = RDX:RAX / r/m, RDX = remainder

                emit(AMD64_MOV32)
                    .add_imm(0)
                    // (0) is explicitly defined in RDX:4, but it's value is expired.
                    .add_reg({ RDX, 4, true, false, true });

                emit(getOpUDiv(type), { dReg })
                    // RAX is implicitly used, and its value expires.
                    .add_reg({ RAX, 8, false, true, true })
                    // RDX is implicitly used, and its value expires.
                    .add_reg({ RDX, 8, false, true, true })
                    // If this is a mod, the remainder is used, and RAX (quotient) expires.
                    .add_reg({ RAX, 8, true, true, is_mod })
                    // If this is a div, the quotient is used, and RDX (remainder) expires.
                    .add_reg({ RDX, 8, true, true, !is_mod });
            }

            if (is_mod) {
                emit(get_move_op(type))
                    // Remainder is explicitly moved from RDX, so it expires.
                    .add_reg({ RDX, get_subreg_byte(type), false, false, true })
                    .add_reg(dReg);
            } else {
                emit(get_move_op(type))
                    // Quotient is explicitly moved from RAX, so it expires.
                    .add_reg({ RAX, get_subreg_byte(type), false, false, true })
                    .add_reg(dReg);
            }

            break;
        }

        case Binop::Op::FAdd: {
            if (rhs.is_imm()) {
                const MachineOperand tmp = lhs;
                lhs = rhs;
                rhs = tmp;
            }

            emit(getOpFAdd(type), { lhs, rhs })
                .add_comment(stringify_inst(B));

            emit(get_move_op(type), { rhs, dReg });

            break;
        }

        case Binop::Op::FSub: {
            if (lhs.is_imm()) {
                emit(get_move_op(type), { lhs, dReg })
                    .add_comment(stringify_inst(B));

                emit(getOpFSub(type), { rhs, dReg });
            } else {
                emit(getOpFSub(type), { rhs, lhs })
                    .add_comment(stringify_inst(B));

                emit(get_move_op(type), { lhs, dReg });
            }
            
            break;
        }

        case Binop::Op::FMul: {
            if (lhs.is_imm()) {
                emit(get_move_op(type), { lhs, dReg })
                    .add_comment(stringify_inst(B));

                emit(getOpFMul(type), { rhs, dReg });
            } else {
                emit(getOpFMul(type), { rhs, lhs })
                    .add_comment(stringify_inst(B));

                emit(get_move_op(type), { lhs, dReg });
            }

            break;
        }

        case Binop::Op::FDiv: {
            if (lhs.is_imm()) {
                emit(get_move_op(type), { lhs, dReg })
                    .add_comment(stringify_inst(B));

                emit(getOpFDiv(type), { rhs, dReg });
            } else {
                emit(getOpFDiv(type), { rhs, lhs })
                    .add_comment(stringify_inst(B));

                emit(get_move_op(type), { lhs, dReg });
            }

            break;
        }

        case Binop::Op::And: {
            if (rhs.is_imm()) {
                const MachineOperand tmp = lhs;
                lhs = rhs;
                rhs = tmp;
            }

            emit(getOpAnd(type), { lhs, rhs })
                .add_comment(stringify_inst(B));

            emit(get_move_op(type), { rhs, dReg });

            break;
        }

        case Binop::Op::Or: {
            if (rhs.is_imm()) {
                const MachineOperand tmp = lhs;
                lhs = rhs;
                rhs = tmp;
            }

            emit(getOpOr(type), { lhs, rhs })
                .add_comment(stringify_inst(B));

            emit(get_move_op(type), { rhs, dReg });

            break;
        }

        case Binop::Op::Xor: {
            if (rhs.is_imm()) {
                const MachineOperand tmp = lhs;
                lhs = rhs;
                rhs = tmp;
            }

            emit(getOpXor(type), { lhs, rhs })
                .add_comment(stringify_inst(B));

            emit(get_move_op(type), { rhs, dReg });

            break;
        }

        case Binop::Op::Shl:
        case Binop::Op::Shr:
        case Binop::Op::Sar: {
            AMD64_Op op;
            if (B->op() == Binop::Op::Shl) {
                op = getOpShl(type);
            } else if (B->op() == Binop::Op::Shr) {
                op = getOpShr(type);
            } else if (B->op() == Binop::Op::Sar) {
                op = getOpSar(type);
            }

            emit(get_move_op(type), { lhs, dReg })
                .add_comment(stringify_inst(B));

            if (rhs.is_imm()) {
                emit(op, { rhs, dReg });
            } else {
                MachineRegister CL = MachineRegister()
                    .setReg(AMD64_Register::RCX)
                    .setSubreg(1)
                    .setIsDef()
                    .setIsExplicit();

                if (rhs.is_reg())
                    rhs.reg().setSubreg(1);

                emit(get_move_op(type), { rhs, CL });
                emit(op, { CL, dReg });
            }

            break;
        }
    }
}

void AMD64LoweringPass::lower_cast(const Cast* C) {
    const Value* value = C->get_value();
    MachineOperand source = to_operand(value);
    MachineRegister dReg = MachineRegister(get_vreg_from_def(C))
        .setSubreg(get_subreg_byte(C->get_type()));

    switch (C->kind()) 
    {
        case Cast::Kind::SExt: {
            AMD64_Op op;
            const uint32_t srcsz = m_mach.get_type_size(value->get_type());
            const uint32_t dstsz = m_mach.get_type_size(C->get_type());

            if (srcsz == 32 && dstsz == 64) {
                op = AMD64_MOVSXD;
            } else {
                op = AMD64_MOVSX;
            }
            
            emit(op, { source, dReg })
                .add_comment(stringify_inst(C));

            break;
        }

        case Cast::Kind::ZExt: {
            AMD64_Op op;
            const uint32_t srcsz = m_mach.get_type_size(value->get_type());
            const uint32_t dstsz = m_mach.get_type_size(C->get_type());

            if (srcsz == 32 && dstsz == 64) {
                // The upper 32 bits are already zero-extended, so we can just make a redundant
                // move that can get cleaned up later.
                op = AMD64_MOV;
                dReg.setSubreg(4);
            } else {
                op = AMD64_MOVSX;
            }

            emit(op, { source, dReg })
                .add_comment(stringify_inst(C));

            break;
        }

        case Cast::Kind::FExt: {
            emit(AMD64_CVTSS2SD, { source, dReg })
                .add_comment(stringify_inst(C));
                
            break;
        }

        case Cast::Kind::ITrunc: {
            if (source.is_reg())
                source.reg().setSubreg(get_subreg_byte(C->get_type()));

            emit(AMD64_MOV, { source, dReg })
                .add_comment(stringify_inst(C));

            break;
        }

        case Cast::Kind::FTrunc: {
            emit(AMD64_CVTSD2SS, { source, dReg })
                .add_comment(stringify_inst(C));

            break;
        }

        case Cast::Kind::S2F: {
            AMD64_Op op;

            if (C->get_type()->is_float_type(32)) {
                op = AMD64_CVTSI2SS;
            } else if (C->get_type()->is_float_type(64)) {
                op = AMD64_CVTSI2SD;
            }

            emit(op, { source, dReg })
                .add_comment(stringify_inst(C));

            break;
        }

        case Cast::Kind::U2F: {
            AMD64_Op op;

            if (C->get_type()->is_float_type(32)) {
                op = AMD64_VCVTUSI2SS;
            } else if (C->get_type()->is_float_type(64)) {
                op = AMD64_VCVTUSI2SD;
            }

            emit(op, { source, dReg })
                .add_comment(stringify_inst(C));

            break;
        }

        case Cast::Kind::F2S: {
            AMD64_Op op;

            if (value->get_type()->is_float_type(32)) {
                op = getOpSS2SI(C->get_type());
            } else if (value->get_type()->is_float_type(64)) {
                op = getOpSD2SI(C->get_type());
            }

            emit(op, { source, dReg })
                .add_comment(stringify_inst(C));

            break;
        }

        case Cast::Kind::F2U: {
            AMD64_Op op;

            if (value->get_type()->is_float_type(32)) {
                op = getOpSS2UI(C->get_type());
            } else if (value->get_type()->is_float_type(64)) {
                op = getOpSD2UI(C->get_type());
            }

            emit(op, { source, dReg })
                .add_comment(stringify_inst(C));

            break;
        } 

        case Cast::Kind::I2P: {
            emit(get_move_op(value->get_type()), { source, dReg })
                .add_comment(stringify_inst(C));

            break;
        }

        case Cast::Kind::P2I:
        case Cast::Kind::Reint: {
            AMD64_Op op;
            if (dynamic_cast<const Local*>(value) || dynamic_cast<const Global*>(value)) {
                op = AMD64_LEA64;
            } else {
                op = AMD64_MOV64;
            }

            emit(op, { source, dReg })
                .add_comment(stringify_inst(C));

            break;
        }
    }
}

void AMD64LoweringPass::lower_cmp(const Cmp* C) {
    MachineOperand left = to_operand(C->get_lhs());
    MachineOperand right = to_operand(C->get_rhs());

    AMD64_Op SETcc = cmp_to_setcc(C->pred());

    if (left.is_imm()) {
        // Left hand side cannot be an immediate, so swap the operands.
        const MachineOperand tmp = left;
        left = right;
        right = tmp;
    } else {
        // Thanks to the lovely AT&T syntax, the operands are technically the other way around by
        // default, so the SETcc needs to compensate for that.
        SETcc = flip_setcc(SETcc);
    }

    emit(get_cmp_op(C->get_lhs()->get_type()), { left, right })
        .add_comment(stringify_inst(C));

    emit(SETcc)
        .add_reg(MachineRegister { get_vreg_from_def(C), 1 });
}
