#include "siir/basicblock.hpp"
#include "siir/function.hpp"
#include "siir/instruction.hpp"

using namespace stm;
using namespace stm::siir;

BasicBlock::BasicBlock(const std::vector<BlockArgument*>& args, 
                       Function* parent, const std::string& name)
    : m_name(name), m_args(args), m_parent(parent) {}

BasicBlock::~BasicBlock() {
    for (auto arg : m_args) delete arg;
    m_args.clear();

    Instruction* curr = m_front;
    while (curr) {
        Instruction* tmp = curr->next();

        curr->set_prev(nullptr);
        curr->set_next(nullptr);
        delete curr;

        curr = tmp;
    }

    m_front = m_back = nullptr;
    m_prev = m_next = nullptr;
    m_preds.clear();
    m_succs.clear();
}

BasicBlock* 
BasicBlock::create(const std::vector<BlockArgument*>& args, 
                   Function* append_to, const std::string& name) {
    BasicBlock* blk = new BasicBlock(args, append_to, name);
    if (append_to)
        append_to->push_back(blk);

    return blk;
}

void BasicBlock::append_to(Function* parent) {
    assert(parent);
    assert(!m_parent && "basic block already belongs to a function");
    
    m_parent = parent;
    parent->push_back(this);
}

void BasicBlock::insert_before(BasicBlock* blk) {
    assert(blk);
    assert(!m_parent && "basic block already belongs to a function");

    m_prev = blk->m_prev;
    m_next = blk;

    if (blk->m_prev)
        blk->m_prev->m_next = this;

    blk->m_prev = this;
    m_parent = blk->m_parent;
}

void BasicBlock::insert_after(BasicBlock* blk) {
    assert(blk);
    assert(!m_parent && "basic block already belongs to a function");

    m_prev = blk;
    m_next = blk->m_next;

    if (blk->m_next)
        blk->m_next->m_prev = this;

    blk->m_next = this;
    m_parent = blk->m_parent;
}

void BasicBlock::remove(Instruction* inst) {
    for (auto curr = m_front; curr; curr = curr->next()) {
        if (curr != inst)
            continue;

        if (curr->prev())
            curr->prev()->set_next(inst->next());

        if (curr->next())
            curr->next()->set_prev(inst->prev());

        inst->set_prev(nullptr);
        inst->set_next(nullptr);
        inst->clear_parent();
    }
}

bool BasicBlock::is_entry() const {
    return m_parent != nullptr && m_prev == nullptr;
}

void BasicBlock::detach() {
    if (m_parent)
        m_parent->remove(this);
    
    m_prev = nullptr;
    m_next = nullptr;
    m_parent = nullptr;
}

void BasicBlock::push_front(Instruction* inst) {
    assert(inst);

    if (m_front) {
        inst->set_next(m_front);
        m_front->set_prev(inst);
        m_front = inst;
    } else {
        m_front = m_back = inst;
    }
}

void BasicBlock::push_back(Instruction* inst) {
    assert(inst);

    if (m_back) {
        inst->set_prev(m_back);
        m_back->set_next(inst);
        m_back = inst;
    } else {
        m_front = m_back = inst;
    }
}

void BasicBlock::insert(Instruction* inst, u32 idx) {
    u32 position = 0;
    for (auto curr = m_front; curr; curr = curr->next()) {
        if (position == idx) {
            inst->insert_before(curr);
            return;
        }
        
        position++;
    }

    push_back(inst);
}

void BasicBlock::insert(Instruction* inst, Instruction* insert_after) {
    inst->insert_after(insert_after);
}

u32 BasicBlock::get_number() const {
    u32 num = 0;
    const BasicBlock* curr = m_prev;
    while (curr) {
        curr = curr->prev();
        num++;
    }

    return num;
}

bool BasicBlock::terminates() const {
    for (auto curr = m_back; curr; curr = curr->prev())
        if (curr->is_terminator()) return true;

    return false;
}

u32 BasicBlock::terminators() const {
    u32 num = 0;
    for (auto curr = m_front; curr; curr = curr->next())
        if (curr->is_terminator()) num++;

    return num;
}

const Instruction* BasicBlock::terminator() const {
    for (auto curr = m_front; curr; curr = curr->next())
        if (curr->is_terminator()) return curr;

    return nullptr;
}
