#ifndef STATIM_SIIR_BASIC_BLOCK_HPP_
#define STATIM_SIIR_BASIC_BLOCK_HPP_

#include "argument.hpp"
#include "instruction.hpp"

#include <cassert>
#include <cstddef>
#include <iterator>
#include <string>
#include <vector>

namespace stm {

class BasicBlock final {
    std::string m_name;
    std::vector<BlockArgument*> m_args;
    Function* m_parent;
    Instruction* m_front;
    Instruction* m_back;
    std::vector<BasicBlock*> m_preds;
    std::vector<BasicBlock*> m_succs;

public:
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

    static BasicBlock* create(
        const std::string& name,
        const std::vector<BlockArgument*> args = {},
        Function* parent = nullptr);

    const std::string& get_name() const { return m_name; }

    const std::vector<BlockArgument*>& get_args() const { return m_args; }

    const BlockArgument* get_arg(u32 i) const {
        assert(i <= num_args());
        return m_args[i];
    }

    BlockArgument* get_arg(u32 i) {
        assert(i <= num_args());
        return m_args[i];
    }

    /// \returns The number of arguments to this basic block.
    u32 num_args() const { return m_args.size(); }

    /// \returns `true` if this basic block has any arguments.
    bool has_args() const { return !m_args.empty(); }

    const Function* get_parent() const { return m_parent; }
    Function* get_parent() { return m_parent; }

    /// Clear the parent link of this block. Does not detach it.
    void clear_parent() { m_parent = nullptr; }

    /// Detach this block from its parent. Does not destroy the block.
    void detach();

    const Instruction* front() const { return m_front; }
    Instruction* front() { return m_front; }

    const Instruction* back() const { return m_back; }
    Instruction* back() { return m_back; }

    /// Prepend \p inst to this block.
    void push_front(Instruction* inst);

    /// Append \p inst to this block.
    void push_back(Instruction* inst);

    /// \returns `true` if this block has no instructions.
    bool empty() const { return m_front == nullptr; }

    /// \returns The size of this block by its instruction count.
    u32 size() const { return std::distance(begin(), end()); }

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

    const std::vector<BasicBlock*>& predecessors() const { return m_preds; }
    const std::vector<BasicBlock*>& succcessors() const { return m_succs; }

    u32 num_predecessors() const { return m_preds.size(); }
    u32 num_successors() const { return m_succs.size(); }

    bool has_predecessors() const { return m_preds.empty(); }
    bool has_successors() const { return m_succs.empty(); }
};

} // namespace stm

#endif // STATIM_SIIR_BASIC_BLOCK_HPP_
