#ifndef STATIM_SIIR_INST_BUILDER_HPP_
#define STATIM_SIIR_INST_BUILDER_HPP_

#include "siir/argument.hpp"
#include "siir/basicblock.hpp"
#include "siir/constant.hpp"
#include "siir/instruction.hpp"
#include "siir/type.hpp"

#include <cassert>

namespace stm {

namespace siir {

class InstBuilder final {
    CFG& m_cfg;
    BasicBlock* m_insert = nullptr;

public:
    InstBuilder(CFG& cfg) : m_cfg(cfg) {}

    const BasicBlock* get_insert() const { return m_insert; }
    BasicBlock* get_insert() { return m_insert; }
    void set_insert(BasicBlock* blk) { m_insert = blk; }

    Value* build_const(Constant* constant, const std::string& name = "") {
        assert(constant);
        return ConstInst::create(constant, name, m_insert);
    }

    StoreInst* build_store(Value* value, Value* dst) {
        return build_store_aligned(value, dst, 0); // use natural alignment
    }

    StoreInst* build_store_aligned(Value* value, Value* dst, u32 align) {
        return StoreInst::create(value, dst, align);
    }

    Value* build_load(const Type* type, Value* src, 
                      const std::string& name = "") {
        return build_load_aligned(type, src, 0, name); // use natural alignment
    }

    Value* build_load_aligned(const Type* type, Value* src, u32 align, 
                              const std::string& name = "") {
        assert(type);
        assert(src);
        return LoadInst::create(src, align, type, name, m_insert);
    }

    Value* build_select(Value* cond, Value* tval, Value* fval, 
                        const std::string& name = "") {
        assert(cond);
        assert(tval);
        assert(fval);
        assert(*cond->get_type() == *Type::get_i1_type(m_cfg));
        assert(*tval->get_type() == *fval->get_type());
        return SelectInst::create(cond, tval, fval, tval->get_type(), name, 
            m_insert);
    }

    BrifInst* build_brif(Value* cond, Value* tdst, Value* fdst) {
        assert(cond);
        assert(tdst);
        assert(fdst);
        assert(*cond->get_type() == *Type::get_i1_type(m_cfg));
        assert(dynamic_cast<BlockAddress*>(tdst));
        assert(dynamic_cast<BlockAddress*>(fdst));
        return BrifInst::create(cond, tdst, fdst, m_insert);
    }

    JmpInst* build_jmp(Value* dst) {
        assert(dst);
        assert(dynamic_cast<BlockArgument*>(dst));
        return JmpInst::create(dst, m_insert);
    }

    RetInst* build_ret(Value* value) {
        return RetInst::create(value, m_insert);
    }

    RetInst* build_ret_void() {
        return build_ret(nullptr);
    }

    AbortInst* build_abort() {
        return AbortInst::create(m_insert);
    }

    UnreachableInst* build_unreachable() {
        return UnreachableInst::create(m_insert);
    }

    Value* build_call(const FunctionType* type, Value* callee, 
                      const std::vector<Value*>& args, 
                      const std::string& name = "") {
        assert(type);
        assert(callee);
        return CallInst::create(callee, args, type->get_return_type(), name, 
            m_insert);
    }

    Value* build_cmp(CmpInst::Predicate pred, Value* left, Value* right,
                     const std::string& name = "") {
        assert(left);
        assert(right);
        return CmpInst::create(pred, left, right, Type::get_i1_type(m_cfg), 
            name, m_insert);
    }

    Value* build_binop(const Type* type, BinopInst::Ops op, Value* left, 
                       Value* right, const std::string& name = "") {
        assert(type);
        assert(left);
        assert(right);
        assert(*left->get_type() == *right->get_type());
        return BinopInst::create(op, left, right, type, name, m_insert);
    }

    Value* build_unop(const Type* type, UnopInst::Ops op, Value *value,
                      const std::string& name = "") {
        assert(type);
        assert(value);
        return UnopInst::create(op, value, type, name, m_insert);
    }
};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_INST_BUILDER_HPP_
