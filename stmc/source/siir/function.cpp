#include "siir/basicblock.hpp"
#include "siir/cfg.hpp"
#include "siir/function.hpp"

using namespace stm;
using namespace stm::siir;

Argument::Argument(const Type* type, const std::string& name, u32 number, 
                   Function* parent)
    : Value(type), m_name(name), m_number(number), m_parent(parent) {}

Function::Function(CFG& cfg, LinkageType linkage, const FunctionType* type,
                   const std::string& name, const std::vector<Argument*>& args)
    : Value(type), m_linkage(linkage), m_name(name), m_args(args) {

    for (u32 idx = 0, e = args.size(); idx != e; ++idx) {
        args[idx]->set_number(idx);
        args[idx]->set_parent(this);
    }

    cfg.add_function(this);
}

Function::~Function() {
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

void Function::detach_from_parent() {
    assert(m_parent);

    m_parent->remove_function(this);
    m_parent = nullptr;
}

const Argument* Function::get_arg(u32 i) const {
    assert(i <= num_args());
    return m_args[i];
}

void Function::set_arg(u32 i, Argument* arg) {
    assert(i <= num_args());
    m_args[i] = arg;
    arg->set_number(i);
    arg->set_parent(this);
}

const Local* Function::get_local(const std::string& name) const {
    auto it = m_locals.find(name);
    if (it != m_locals.end())
        return it->second;

    return nullptr;
}

void Function::add_local(Local* local) {
    assert(!get_local(local->get_name()) &&
        "local with name already exists in function");
    
    m_locals.emplace(local->get_name(), local);
    local->set_parent(this);
}

void Function::remove_local(Local* local) {
    assert(local && "local cannot be null");

    auto it = m_locals.find(local->get_name());
    if (it == m_locals.end())
        return;

    m_locals.erase(it);
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
