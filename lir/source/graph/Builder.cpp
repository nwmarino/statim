//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/CFG.h"
#include "lir/graph/Builder.h"
#include "lir/graph/Constant.h"
#include "lir/graph/Instruction.h"

using namespace lir;

void Builder::insert(Instruction *inst) {
    if (!m_insert)
        return;

    switch (m_mode) {
        case InsertMode::Prepend:
            m_insert->prepend(inst);
            break;
        case InsertMode::Append:
            m_insert->append(inst);
            break;
    }
}

Const *Builder::build_const(Constant *value) {
    assert(value && "value cannot be null!");

    Const *inst = new Const(
        value->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Const *Builder::build_string(String *string) {
    assert(string && "string cannot be null!");

    Const *inst = new Const(
        string->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        string);
    assert(inst);

    insert(inst);
    return inst;
}

Const *Builder::build_agg(Aggregate *agg) {
    assert(agg && "aggregate cannot be null!");

    Const *inst = new Const(agg->get_type(), nullptr, m_cfg.get_def_id(), agg);
    assert(inst);

    insert(inst);
    return inst;
}

Load *Builder::build_load(Type *type, Value *ptr) {
    assert(type && "type cannot be null!");
    assert(ptr && "ptr cannot be null!");
    assert(ptr->get_type()->is_pointer_type() && "ptr must be a pointer!");

    const uint16_t align = (m_cfg.get_machine().get_type_align(type) / 8);

    Load *inst = new Load(type, nullptr, m_cfg.get_def_id(), ptr, align);
    assert(inst);

    insert(inst);
    return inst;
}

Store *Builder::build_store(Value *value, Value *ptr) {
    assert(value && "value cannot be null!");
    assert(ptr && "ptr cannot be null!");
    assert(ptr->get_type()->is_pointer_type() && "ptr must be a pointer!");
    
    const uint16_t align = 
        (m_cfg.get_machine().get_type_align(value->get_type()) / 8);

    Store *inst = new Store(nullptr, value, ptr, align);
    assert(inst);

    insert(inst);
    return inst;
}

Access *Builder::build_access(Type *type, Value *ptr, Value *index) {
    assert(type && "type cannot be null!");
    assert(ptr && "ptr cannot be null!");
    assert(ptr->get_type()->is_pointer_type() && "ptr must be a pointer!");
    assert(index && "index cannot be null!");
    assert(index->get_type()->is_integer_type() && "index must be an integer!");

    Access *inst = new Access(type, nullptr, m_cfg.get_def_id(), ptr, index);
    assert(inst);

    insert(inst);
    return inst;
}

Offptr *Builder::build_offptr(Type *type, Value *ptr, Value *index) {
    assert(type && "type cannot be null!");
    assert(ptr && "ptr cannot be null!");
    assert(ptr->get_type()->is_pointer_type() && "ptr must be a pointer!");
    assert(index && "index cannot be null!");
    assert(index->get_type()->is_integer_type() && "index must be an integer!");

    Offptr *inst = new Offptr(type, nullptr, m_cfg.get_def_id(), ptr, index);
    assert(inst);

    insert(inst);
    return inst;
}

Call *Builder::build_call(Function *callee, const std::vector<Value*> &args) {
    assert(callee && "callee cannot be null!");
    assert(args.size() == callee->num_params() && "argument count mismatch!");

    Type *result = callee->get_type()->get_result();

    std::vector<Value*> ops = { callee };
    for (Value *arg : args)
        ops.push_back(arg);

    Call *inst = new Call(
        callee->get_type()->get_result(), 
        nullptr, 
        result->is_void_type() ? 0 : m_cfg.get_def_id(), 
        ops);
    assert(inst);

    insert(inst);
    return inst;
}

Ret *Builder::build_ret(Value *value) {
    Ret *inst = new Ret(nullptr, value);
    assert(inst);

    insert(inst);
    return inst;
}

Jump *Builder::build_jump(BasicBlock *dest) {
    assert(dest && "dest cannot be null!");

    Jump *inst = new Jump(nullptr);
    assert(inst);

    m_insert->add_succ(dest);
    dest->add_pred(m_insert);

    insert(inst);
    return inst;
}

Brif *Builder::build_brif(Value *cond, BasicBlock *tdest, BasicBlock *fdest) {
    assert(cond && "cond cannot be null!");
    assert(cond->get_type()->is_integer_type(8) && "cond must be a boolean!");
    assert(tdest && "tdest cannot be null!");
    assert(fdest && "fdest cannot be null!");

    m_insert->add_succ(tdest);
    tdest->add_pred(m_insert);

    m_insert->add_succ(fdest);
    fdest->add_pred(m_insert);

    Brif *inst = new Brif(nullptr, cond);
    assert(inst);

    insert(inst);
    return inst;
}

Phi *Builder::build_phi(Type *type) {
    assert(type && "type cannot be null!");

    Phi *node = new Phi(type, nullptr, m_cfg.get_def_id());
    assert(node);

    insert(node);
    return node;
}

Unop *Builder::build_not(Value *value) {
    assert(value && "value cannot be null!");

    Unop *inst = new Unop(
        value->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Unop::Op::Not, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Unop *Builder::build_ineg(Value *value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");

    Unop *inst = new Unop(
        value->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Unop::Op::INeg, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Unop *Builder::build_fneg(Value *value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_float_type() && "value must be a float!");

    Unop *inst = new Unop(
        value->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Unop::Op::FNeg, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_iadd(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::IAdd, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_isub(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::ISub, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_imul(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::IMul, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_sdiv(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::SDiv, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_udiv(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::UDiv, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_smod(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::SMod, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_umod(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::UMod, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_fadd(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::FAdd, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_fsub(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::FSub, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_fmul(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::FMul,
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_fdiv(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::FDiv, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_and(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::And, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_or(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::Or, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_xor(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::Xor, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_shl(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::Shl, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_shr(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::Shr, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Binop *Builder::build_sar(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Binop *inst = new Binop(
        lhs->get_type(), 
        nullptr, 
        m_cfg.get_def_id(), 
        Binop::Op::Sar, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_sext(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_integer_type() && "type must be an integer!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::SExt, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_zext(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_integer_type() && "type must be an integer!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::ZExt, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_fext(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_float_type() && "type must be a float!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_float_type() && "value must be a float!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::FExt, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_itrunc(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_integer_type() && "type must be an integer!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::ITrunc, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_ftrunc(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_float_type() && "type must be a float!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_float_type() && "value must be a float!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::FTrunc, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_s2f(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_float_type() && "type must be a float!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::S2F, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_u2f(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_float_type() && "type must be a float!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::S2F, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_f2s(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_integer_type() && "type must be an integer!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_float_type() && "value must be a float!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::F2S, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_f2u(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_integer_type() && "type must be an integer!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_float_type() && "value must be a float!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::F2U, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_p2i(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_integer_type() && "type must be an integer!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_pointer_type() && "value must be a pointer!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::P2I, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_i2p(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_pointer_type() && "type must be a pointer!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::I2P, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cast *Builder::build_reint(Type *type, Value *value) {
    assert(type && "type cannot be null!");
    assert(type->is_pointer_type() && "type must be a pointer!");
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_pointer_type() && "value must be a pointer!");

    Cast *inst = new Cast(
        type, 
        nullptr, 
        m_cfg.get_def_id(), 
        Cast::Kind::Reint, 
        value);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_ieq(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::IEq, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_ine(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::INe, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_slt(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Slt, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_sle(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Sle, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_sgt(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Sgt, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_sge(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Sge, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_ult(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Ult, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_ule(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Ule, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_ugt(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Ugt, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_uge(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Uge, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_feq(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::FEq, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_fne(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::FNe, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_flt(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Flt, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_fle(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Fle, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_fgt(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Fgt, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}

Cmp *Builder::build_cmp_fge(Value *lhs, Value *rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs && "rhs cannot be null!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && "operand types must match!");

    Cmp *inst = new Cmp(
        Type::get_i8(m_cfg), 
        nullptr, 
        m_cfg.get_def_id(), 
        Cmp::Predicate::Fge, 
        lhs, 
        rhs);
    assert(inst);

    insert(inst);
    return inst;
}
