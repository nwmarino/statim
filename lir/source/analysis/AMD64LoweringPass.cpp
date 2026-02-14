//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/analysis/AMD64LoweringPass.hpp"
#include "lir/graph/Instruction.hpp"
#include "lir/machine/AMD64.hpp"
#include "lir/machine/MachineFunction.hpp"
#include "lir/machine/MachineOp.hpp"
#include "lir/machine/Register.hpp"

#include <string>
#include <sstream>
#include <unordered_map>

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
    for (const Global *global : m_cfg.get_globals()) {
        MachineData::Data bytes = {};
        
        if (global->has_initializer()) {
            lower_constant(global->get_initializer(), bytes);
        } else {
            const auto ptr = dynamic_cast<PointerType*>(global->get_type());
            assert(ptr);

            const uint32_t bits = 
                m_obj.get_machine().get_type_size(ptr->get_pointee());
            bytes.push_back(MachineConstant(bits / 8));
        }

        MachineData *MD = new MachineData(
            global->get_name(), 
            bytes,
            global->has_linkage(Global::LinkageType::Public), 
            !global->is_mutable());
        assert(MD);

        m_obj.get_globals().emplace(global->get_name(), MD);
    }

    // Lower each function and it's blocks to machine functions & labels.
    for (const Function *func : m_cfg.get_functions()) {
        // Empty functions should not be lowered, they should either be
        // resolved at link time or with some library.
        if (func->empty())
            continue;

        FunctionABI abi = FunctionABI(m_mach, func);

        MachineFunction *MF = new MachineFunction(&m_obj, abi, func->get_name());
        assert(MF);

        const BasicBlock *curr = func->get_head();
        while (curr) {
            MachineLabel *ML = new MachineLabel(MF);
            assert(ML);

            curr = curr->get_next();
        }
    }

    // Properly lower the instruction list of each function, on a block basis.
    for (const Function *func : m_cfg.get_functions()) {
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

AMD64_Op AMD64LoweringPass::get_sized_op(const Type *type, 
                                         const std::array<AMD64_Op, 4> &gp) {
    if (type->is_integer_type() || type->is_pointer_type()) {
        static const std::unordered_map<uint32_t, AMD64_Op> table = {
            { 8, gp[0] }, { 16, gp[1] }, { 32, gp[2] }, { 64, gp[3] }
        };

        return table.at(m_mach.get_type_size(type));
    } else {
        assert(false && "(1) non-integer op!");
    }
}

AMD64_Op AMD64LoweringPass::get_sized_op(const Type *type, 
                                         const std::array<AMD64_Op, 4> &gp, 
                                         const std::array<AMD64_Op, 2> &fp) {
    if (type->is_integer_type() || type->is_pointer_type()) {
        return get_sized_op(type, gp);
    } else if (type->is_float_type()) {
        static const std::unordered_map<uint32_t, AMD64_Op> table = {
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
        return MachineOperand(0L);
    } else if (auto inst = dynamic_cast<const Instruction*>(value)) {
        Register vreg = get_vreg_from_def(inst);
        uint8_t subreg = get_subreg_byte(value->get_type());

        return MachineOperand(MachineRegister { vreg, subreg });
    } else if (auto param = dynamic_cast<const Parameter*>(value)) {
        const FunctionABI &abi = m_func->abi();
        const uint32_t index = param->get_index();

        const FunctionABI::Location &loc = abi.get_param_location(index);
        MachineRegister SP(RSP, 8);
        
        return MachineOperand(Memory { SP, loc.offset });
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
    } else if (auto E = dynamic_cast<const Extract*>(inst)) {
        lower_extract(E);
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

    MachineRegister dest(get_vreg_from_def(C), get_subreg_byte(C->get_type()));

    if (auto integer = dynamic_cast<const Integer*>(constant)) {
        // MOVABS lets us put a 64-bit integer into a register.
        emit(AMD64_MOVABS)
            .add_reg(dest)
            .add_imm(integer->get_value())
            .add_comment(stringify_inst(C));
    } else if (auto fp = dynamic_cast<const Float*>(constant)) {
        // Floats need to be pooled within the local function.
        MachineData::Data bytes = {};
        lower_constant(fp, bytes);

        // Materialie a new floating point constant from the function pool.
        ConstantPool &pool = m_func->get_pool();
        MachineData *MD = pool.materialize(bytes);

        emit(get_move_op(fp->get_type()))
            .add_reg(dest)
            .add_data(MD)
            .add_comment(stringify_inst(C));
    } else if (auto string = dynamic_cast<const String*>(constant)) {
        // Strings must also be pooled into the local function.
        MachineData::Data bytes = {};
        lower_constant(string, bytes);

        ConstantPool &pool = m_func->get_pool();
        MachineData *MD = pool.materialize(bytes);

        emit(AMD64_LEA64)
            .add_reg(dest)
            .add_data(MD)
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

    emit(get_move_op(value->get_type()), { source, dest })
        .add_comment(stringify_inst(S));
}

void AMD64LoweringPass::lower_access(const Access *A) {
    const MachineOperand source = to_operand(A->get_base());
    const MachineOperand index = to_operand(A->get_index());

    auto structure = dynamic_cast<const StructType*>(A->get_base()->get_type());
    assert(structure);

    // @Todo: Assumes immediate index.
    uint32_t offset = m_mach.get_field_offset(structure, index.imm());

    MachineRegister MR(get_vreg_from_def(A), get_subreg_byte(A->get_type()));

    emit(AMD64_MOV64, { source })
        .add_reg(MR)
        .add_comment(stringify_inst(A));

    emit(AMD64_ADD64)
        .add_imm(offset)
        .add_reg(MR);
}

void AMD64LoweringPass::lower_extract(const Extract *E) {
    // @Todo: Assess.
    const MachineOperand source = to_operand(E->get_base());

    if (source.is_reg()) {
        // If the structure is in a register, it must be 8 bytes in size or
        // less.
        assert(false && "(2) non-scalar op!");
    }

    auto structure = dynamic_cast<const StructType*>(E->get_base()->get_type());
    assert(structure);

    uint32_t offset = m_mach.get_field_offset(structure, E->get_index());

    MachineRegister MR(get_vreg_from_def(E), get_subreg_byte(E->get_type()));

    emit(get_move_op(E->get_type()), { source })
        .add_reg(MR)
        .add_comment(stringify_inst(E));
}

void AMD64LoweringPass::lower_offptr(const Offptr *O) {
    const MachineOperand source = to_operand(O->get_base());
    const MachineOperand index = to_operand(O->get_index());

    if (index.is_imm()) {
        auto ptr = dynamic_cast<const PointerType*>(O->get_base()->get_type());
        assert(ptr);

        uint32_t bytes = m_mach.get_type_size(ptr->get_pointee()) / 8;
        

    } else {

    }

}

void AMD64LoweringPass::lower_call(const Call *C) {

}

void AMD64LoweringPass::lower_ret(const Ret *R) {
    if (R->has_value()) {
        const FunctionABI &abi = m_func->abi();
        assert(abi.has_result());

        const FunctionABI::Location &loc = abi.get_result_location();
        const Value *value = R->get_value();
        const MachineOperand result = to_operand(value);

        emit(get_move_op(value->get_type()), { result })
            .add_mem(MachineRegister(RBP, 8), loc.offset)
            .add_comment(stringify_inst(R));

        const StackFrame &frame = m_func->get_stack_frame();
        const uint32_t bytes = frame.size();

        emit(static_cast<uint32_t>(Intrinsic::Stack_Restore));
    } else {
        emit(static_cast<uint32_t>(Intrinsic::Stack_Restore))
            .add_comment(stringify_inst(R));
    }

    emit(AMD64_RET64);
}

void AMD64LoweringPass::lower_jump(const Jump *J) {
    emit(AMD64_JMP)
        .add_label(m_func->get_label(J->get_dest()->position()))
        .add_comment(stringify_inst(J));
}

void AMD64LoweringPass::lower_brif(const Brif *B) {
    MachineOperand cond = to_operand(B->get_cond());

    if (cond.is_reg()) {
        // If the condition is coming from a register, make sure it's using 
        // only the lowest byte.
        cond.reg().set_subreg(1);
    }

    emit(AMD64_CMP8)
        .add_imm(0)
        .add_operand(cond)
        .add_comment(stringify_inst(B));

    emit(AMD64_JNE)
        .add_label(m_func->get_label(B->get_true_dest()->position()));

    emit(AMD64_JMP)
        .add_label(m_func->get_label(B->get_false_dest()->position()));
}

void AMD64LoweringPass::lower_phi(const Phi *P) {

}

void AMD64LoweringPass::lower_unop(const Unop *U) {

}

void AMD64LoweringPass::lower_binop(const Binop *B) {

}

void AMD64LoweringPass::lower_cast(const Cast *C) {

}

void AMD64LoweringPass::lower_cmp(const Cmp *C) {
    MachineOperand LHS = to_operand(C->get_lhs());
    MachineOperand RHS = to_operand(C->get_rhs());

    AMD64_Op SETcc = cmp_to_setcc(C->pred());

    if (RHS.is_imm()) {
        // RHS cannot be an immediate, so swap the operands.
        const MachineOperand temp = LHS;
        LHS = RHS;
        RHS = temp;
    } else {
        // Thanks to the lovely AT&T syntax, the operands are technically the
        // other way around, so the SETcc needs to compensate for that.
        SETcc = flip_setcc(SETcc);
    }

    emit(get_cmp_op(C->get_lhs()->get_type()), { LHS, RHS })
        .add_comment(stringify_inst(C));

    const MachineRegister dest(get_vreg_from_def(C), 1);
    emit(SETcc)
        .add_reg(dest);
}
