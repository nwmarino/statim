#include "siir/basicblock.hpp"
#include "siir/cfg.hpp"
#include "siir/function.hpp"

using namespace stm;
using namespace stm::siir;

Function::Function(LinkageTypes linkage, 
                   const std::vector<FunctionArgument*>& args, CFG* parent, 
                   const Type* type, const std::string& name)
    : Value(type, name), m_linkage(linkage), m_args(args), m_parent(parent) {}

Function::~Function() {
    detach();

    for (auto arg : m_args) delete arg;
    m_args.clear();

    for (auto [ name, local ] : m_locals) delete local;
    m_locals.clear();

    BasicBlock* curr = m_front;
    while (curr) {
        BasicBlock* tmp = curr->next();

        curr->set_prev(nullptr);
        curr->set_next(nullptr);
        delete curr;

        curr = tmp;
    }

    m_front = m_back = nullptr;
}

Function* Function::create(const FunctionType* type, LinkageTypes linkage, 
                        const std::vector<FunctionArgument*>& args,
                        CFG* parent, const std::string& name) {
    Function* fn = new Function(linkage, args, parent, type, name);
    if (parent)
        parent->add_function(fn);

    return fn;
}

void Function::detach() {
    assert(m_parent);

    m_parent->remove_function(this);
    clear_parent();
}

void Function::push_front(BasicBlock* blk) {
    assert(blk);

    if (m_front) {
        blk->set_next(m_front);
        m_front->set_prev(blk);
        m_front = blk;
    } else {
        m_front = m_back = blk;
    }
}

void Function::push_back(BasicBlock* blk) {
    assert(blk);

    if (m_back) {
        blk->set_prev(m_back);
        m_back->set_next(blk);
        m_back = blk;
    } else {
        m_front = m_back = blk;
    }
}

void Function::insert(BasicBlock* blk, u32 idx) {
    u32 position = 0;
    for (auto curr = m_front; curr; curr = curr->next()) {
        if (position == idx) {
            blk->insert_before(curr);
            return;
        }

        position++;
    }

    push_back(blk);
}

void Function::insert(BasicBlock* blk, BasicBlock* insert_after) {
    blk->insert_after(insert_after);
}

void Function::remove(BasicBlock* blk) {
    for (auto curr = m_front; curr; curr = curr->next()) {
        if (curr != blk)
            continue;

        if (curr->prev())
            curr->prev()->set_next(blk->next());

        if (curr->next())
            curr->next()->set_prev(blk->prev());

        blk->set_prev(nullptr);
        blk->set_next(nullptr);
        blk->clear_parent();
    } 
}
