#include "siir/basicblock.hpp"
#include "siir/instruction.hpp"

using namespace stm;
using namespace stm::siir;

Instruction::Instruction(std::initializer_list<Value*> operands, 
                         BasicBlock* parent, const Type* type, 
                         const std::string& name)
    : User(operands, type, name), m_parent(parent) {

    if (parent)
        parent->push_back(this);
}

void Instruction::append_to(BasicBlock* blk) {
    assert(blk);
    assert(!m_parent && "instruction already belongs to a block");

    blk->push_back(this);
    m_parent = blk;
}

void Instruction::insert_before(Instruction* inst) {
    assert(inst);
    assert(!m_parent && "instruction already belongs to a block");

    m_prev = inst->m_prev;
    m_next = inst;

    if (inst->m_prev)
        inst->m_prev->m_next = this;

    inst->m_prev = this;
    m_parent = inst->m_parent;
}

void Instruction::insert_after(Instruction* inst) {
    assert(inst);
    assert(!m_parent && "instruction already belongs to a block");

    m_prev = inst;
    m_next = inst->m_next;

    if (inst->m_next)
        inst->m_next->m_prev = this;

    inst->m_next = this;
    m_parent = inst->m_parent;
}

void Instruction::detach() {
    if (m_parent)
        m_parent->remove(this);

    m_prev = nullptr;
    m_next = nullptr;
    m_parent = nullptr;
}

ConstInst::ConstInst(Constant* constant, const std::string& name, 
                     BasicBlock* append_to)
    : Instruction({ constant }, append_to, constant->get_type(), name) {}

ConstInst* 
ConstInst::create(Constant* constant, const std::string& name,
                  BasicBlock* append_to) {
    return new ConstInst(constant, name, append_to);
}

StoreInst::StoreInst(Value* value, Value* dst, u32 align, 
                     BasicBlock* append_to)
    : Instruction({ value, dst }, append_to), m_value(value), m_dst(dst), 
      m_align(align) {}

StoreInst* 
StoreInst::create(Value* value, Value* dst, u32 align, BasicBlock* append_to) {
    return new StoreInst(value, dst, align, append_to);
}

LoadInst::LoadInst(Value* src, u32 align, const Type* type, 
                   const std::string& name, BasicBlock* append_to)
    : Instruction({ src }, append_to), m_src(src), m_align(align) {}

LoadInst* 
LoadInst::create(Value* src, u32 align, const Type* type,
                 const std::string& name, BasicBlock* append_to) {
    return new LoadInst(src, align, type, name, append_to);
}

SelectInst::SelectInst(Value* cond, Value* tval, Value* fval, const Type* type,
                       const std::string& name, BasicBlock* append_to)
    : Instruction({ cond, tval, fval }, append_to, type, name), m_cond(cond), 
      m_tval(tval), m_fval(fval) {}

SelectInst* 
SelectInst::create(Value* cond, Value* tval, Value* fval, const Type* type,
                   const std::string& name, BasicBlock* append_to) {
    return new SelectInst(cond, tval, fval, type, name, append_to);
}

BrifInst::BrifInst(Value* cond, Value* tdst, Value* fdst, BasicBlock* append_to)
    : Instruction({ cond, tdst, fdst }, append_to), m_cond(cond), m_tdst(tdst),
      m_fdst(fdst) {}

BrifInst* 
BrifInst::create(Value* cond, Value* tdst, Value* fdst, BasicBlock* append_to) {
    return new BrifInst(cond, tdst, fdst, append_to);
}

JmpInst::JmpInst(Value* dst, BasicBlock* append_to)
    : Instruction({ dst }, append_to), m_dst(dst) {}

JmpInst* JmpInst::create(Value* dst, BasicBlock* append_to) {
    return new JmpInst(dst, append_to);
}

RetInst::RetInst(Value* value, BasicBlock* append_to)
    : Instruction({ value }, append_to), m_value(value) {}

RetInst* RetInst::create(Value* value, BasicBlock* append_to) {
    return new RetInst(value, append_to);
}

AbortInst::AbortInst(BasicBlock* append_to) : Instruction({}, append_to) {}

AbortInst* AbortInst::create(BasicBlock* append_to) {
    return new AbortInst(append_to);
}

UnreachableInst::UnreachableInst(BasicBlock* append_to) 
    : Instruction({}, append_to) {}

UnreachableInst* UnreachableInst::create(BasicBlock* append_to) {
    return new UnreachableInst(append_to);
}

CallInst::CallInst(Value* callee, const std::vector<Value*>& args, 
                   const Type* type, const std::string& name, 
                   BasicBlock* append_to)
    : Instruction({ callee }, append_to, type, name), m_callee(callee), 
      m_args(args) {} 

CallInst* 
CallInst::create(Value* callee, const std::vector<Value*>& args,
                 const Type* type, const std::string& name, 
                 BasicBlock* append_to) {
    return new CallInst(callee, args, type, name, append_to);
}

CmpInst::CmpInst(Predicate pred, Value* left, Value* right, const Type* type, 
                 const std::string& name, BasicBlock* append_to)
    : Instruction({ left, right }, append_to, type, name), m_pred(pred),
      m_left(left), m_right(right) {}

CmpInst* 
CmpInst::create(Predicate pred, Value* left, Value* right, const Type* type, 
                const std::string& name, BasicBlock* append_to) {
    return new CmpInst(pred, left, right, type, name, append_to);
}

BinopInst::BinopInst(Ops op, Value* left, Value* right, const Type* type, 
                     const std::string& name, BasicBlock* append_to)
    : Instruction({ left, right }, append_to, type, name), m_op(op), 
      m_left(left), m_right(right) {}

BinopInst* 
BinopInst::create(Ops op, Value* left, Value* right, const Type* type, 
                  const std::string& name, BasicBlock* append_to) {
    return new BinopInst(op, left, right, type, name, append_to);
}

UnopInst::UnopInst(Ops op, Value* value, const Type* type, 
                   const std::string& name, BasicBlock* append_to)
    : Instruction({ value }, append_to, type, name), m_op(op), m_value(value) {}

UnopInst* 
UnopInst::create(Ops op, Value* value, const Type* type, 
                 const std::string& name, BasicBlock* append_to) {
    return new UnopInst(op, value, type, name, append_to);
}
