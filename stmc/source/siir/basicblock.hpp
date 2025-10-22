#ifndef STATIM_SIIR_BASIC_BLOCK_HPP_
#define STATIM_SIIR_BASIC_BLOCK_HPP_

#include "siir/instruction.hpp"

#include <cassert>
#include <cstddef>
#include <iterator>
#include <vector>

namespace stm {
namespace siir {

class Function;

/// The BasicBlock type is implemented as a node in a linked list, managed by
/// the Function class. Each block manages a similar doubly linked list of
/// instructions, expecting one and only one terminating instruction at a time.

/// A basic block consisting of instructions.
class BasicBlock final {
    Function* m_parent;
    BasicBlock* m_prev = nullptr;
    BasicBlock* m_next = nullptr;
    Instruction* m_front = nullptr;
    Instruction* m_back = nullptr;
    std::vector<BasicBlock*> m_preds = {};
    std::vector<BasicBlock*> m_succs = {};

public:
    /// Create a new basic block. If the |parent| argument is provided, the new
    /// block will be automatically append to it.
    BasicBlock(Function* parent = nullptr);
    
    BasicBlock(const BasicBlock&) = delete;
    BasicBlock& operator = (const BasicBlock&) = delete;
    
    ~BasicBlock();

    /// Returns the parent function of this basic block.
    const Function* get_parent() const { return m_parent; }
    Function* get_parent() { return m_parent; }

    /// Clear the parent link of this basic block. Does not detach this block
    /// from its old parent.
    void clear_parent() { m_parent = nullptr; }

    /// Mutate the parent link of this basic block to |parent|.
    void set_parent(Function* parent) { m_parent = parent; }

    /// Append this basic block to function |parent|. Assumes this block is 
    /// unlinked and free-floating.
    void append_to_function(Function* parent);

    /// Insert this basic block into the position before |blk|.
    void insert_before(BasicBlock* blk);

    /// Insert this basic block into the position after |blk|.
    void insert_after(BasicBlock* blk);

    /// Remove the instruction |inst| if it belongs to this basic block.
    void remove_inst(Instruction* inst);

    /// Returns true if this basic block has a parent function and is the first
    /// block in that function.
    bool is_entry_block() const { 
        return m_parent != nullptr && m_prev == nullptr; 
    }

    /// Detach this basic block from its parent. Does not destroy the block.
    void detach_from_parent();

    /// Returns the basic block previous to this one in the parent function.
    const BasicBlock* prev() const { return m_prev; }
    BasicBlock* prev() { return m_prev; }

    /// Returns the basic block after this one in the parent function.
    const BasicBlock* next() const { return m_next; }
    BasicBlock* next() { return m_next; }

    void set_prev(BasicBlock* blk) { m_prev = blk; }
    void set_next(BasicBlock* blk) { m_next = blk; }

    /// Returns the first instruction in this block, if one exists.
    const Instruction* front() const { return m_front; }
    Instruction* front() { return m_front; }

    /// Returns the last instruction in this block, if it exists.
    const Instruction* back() const { return m_back; }
    Instruction* back() { return m_back; }

    /// Set the first instruction in this block to |inst|.
    void set_front(Instruction* inst) {
        assert((!inst || inst->get_parent() == this) &&
            "cannot make a stranger instruction the front of this block");
        m_front = inst;
    }

    /// Set the last instruction in this block to |inst|.
    void set_back(Instruction* inst) {
        assert((!inst || inst->get_parent() == this) &&
            "cannot make a stranger instruction the back of this block");
        m_back = inst;
    }

    /// Prepend |inst| to this basic block.
    void push_front(Instruction* inst);

    /// Append |inst| to this basic block.
    void push_back(Instruction* inst);

    /// Insert |inst| into this basic block at position |i|.
    void insert(Instruction* inst, u32 i);

    /// Insert |inst| into this basic block immediately after |insert_after|. 
    /// Fails if |insert_after| is not already inside this block.
    void insert(Instruction* inst, Instruction* insert_after);

    /// Retuns true if this basic block has no instructions.
    bool empty() const { return m_front == nullptr; }

    /// Returns the size of this basic block by its instruction count.
    u32 size() const { return std::distance(begin(), end()); }

    /// Returns the numeric position of this basic block relative to other
    /// blocks in the parent function.
    u32 get_number() const;

    /// Returns the predecessors of this basic block.
    const std::vector<BasicBlock*>& preds() const { return m_preds; }
    std::vector<BasicBlock*>& preds() { return m_preds; }

    /// Returns the number of predecessors to this basic block.
    u32 num_preds() const { return m_preds.size(); }

    /// Returns true if this basic block has atleast one predecessor.
    bool has_preds() const { return !m_preds.empty(); }

    /// Returns the successors of this basic block.
    const std::vector<BasicBlock*>& succs() const { return m_succs; }
    std::vector<BasicBlock*>& succs() { return m_succs; }

    /// Returns the number of successors to this basic block.
    u32 num_succs() const { return m_succs.size(); }
    
    /// Returns true if this basic block has atleast one successor.
    bool has_succs() const { return !m_succs.empty(); }

    /// Returns true if this basic block contains a terminating instruction at 
    /// any point.
    bool terminates() const;

    /// Returns the number of terminating instructions in this basic block.
    u32 terminators() const;
    
    /// The earliest terminating instruction in this basic block if one exists.
    const Instruction* terminator() const;
    Instruction* terminator() {
        return const_cast<Instruction*>(
            static_cast<const BasicBlock*>(this)->terminator());
    }

    /// Print this basic block in a reproducible plaintext format to the output
    /// stream |os|.
    void print(std::ostream& os) const;

    struct iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Instruction;
        using difference_type = std::ptrdiff_t;
        using pointer = Instruction*;
        using reference = Instruction&;

        pointer ptr = nullptr;

        iterator() = default;
        explicit iterator(pointer p) : ptr(p) {}

        reference operator * () const { return *ptr; }
        pointer operator -> () const { return ptr; }

        iterator& operator ++ () { 
            ptr = ptr->next(); 
            return *this; 
        }

        iterator operator ++ (i32) { 
            iterator tmp = *this; 
            ++(*this); 
            return tmp; 
        }

        iterator& operator -- () { 
            ptr = ptr->prev(); 
            return *this; 
        }

        iterator operator -- (i32) { 
            iterator tmp = *this; 
            --(*this); 
            return tmp; 
        }

        bool operator == (const iterator& other) const { 
            return ptr == other.ptr; 
        }

        bool operator != (const iterator& other) const { 
            return ptr != other.ptr; 
        }
    };

    struct const_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const Instruction;
        using difference_type = std::ptrdiff_t;
        using pointer = const Instruction*;
        using reference = const Instruction&;

        const Instruction* ptr = nullptr;

        const_iterator() = default;
        const_iterator(const iterator& it) : ptr(it.ptr) {}
        explicit const_iterator(const Instruction* p) : ptr(p) {}

        reference operator * () const { return *ptr; }
        pointer operator -> () const { return ptr; }

        const_iterator& operator ++ () { 
            ptr = ptr->next(); 
            return *this; 
        }

        const_iterator operator ++ (i32) { 
            auto tmp = *this; 
            ++(*this); 
            return tmp; 
        }

        const_iterator& operator -- () { 
            ptr = ptr->prev(); 
            return *this; 
        }

        const_iterator operator -- (i32) { 
            auto tmp = *this; 
            --(*this); 
            return tmp; 
        }

        bool operator == (const const_iterator& other) const { 
            return ptr == other.ptr; 
        }

        bool operator != (const const_iterator& other) const { 
            return ptr != other.ptr; 
        }
    };

    iterator begin() { return iterator(m_front); }
    iterator end() { return iterator(nullptr); }

    const_iterator begin() const { return const_iterator(m_front); }
    const_iterator end() const { return const_iterator(nullptr); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    auto rbegin() { return std::reverse_iterator(end()); }
    auto rend() { return std::reverse_iterator(begin()); }

    auto rbegin() const { return std::reverse_iterator(end()); }
    auto rend() const { return std::reverse_iterator(begin()); }

    auto crbegin() const { return rbegin(); }
    auto crend() const { return rend(); }
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_BASIC_BLOCK_HPP_
