#ifndef STATIM_SIIR_FUNCTION_HPP_
#define STATIM_SIIR_FUNCTION_HPP_

#include "siir/argument.hpp"
#include "siir/basicblock.hpp"
#include "siir/local.hpp"
#include "siir/value.hpp"

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace stm {

namespace siir {

class CFG;

class Function final : public Value {
public:
    enum LinkageTypes : u8 {
        Internal, External
    };

private:
    CFG* m_parent;
    LinkageTypes m_linkage;
    std::vector<FunctionArgument*> m_args;
    std::map<std::string, Local*> m_locals;
    BasicBlock* m_front;
    BasicBlock* m_back;

    Function(LinkageTypes linkage, const std::vector<FunctionArgument*>& args,
             CFG* parent, const Type* type, const std::string& name);

public:
    struct iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = BasicBlock;
        using difference_type = std::ptrdiff_t;
        using pointer = BasicBlock*;
        using reference = BasicBlock&;

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
        using value_type = const BasicBlock;
        using difference_type = std::ptrdiff_t;
        using pointer = const BasicBlock*;
        using reference = const BasicBlock&;

        const BasicBlock* ptr = nullptr;

        const_iterator() = default;
        const_iterator(const iterator& it) : ptr(it.ptr) {}
        explicit const_iterator(const BasicBlock* p) : ptr(p) {}

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

    ~Function() override;

    static Function* create(const FunctionType* type, LinkageTypes linkage, 
                            const std::vector<FunctionArgument*>& args,
                            CFG* parent = nullptr, const std::string& name = "");

    LinkageTypes get_linkage() const { return m_linkage; }
    void set_linkage(LinkageTypes linkage) { m_linkage = linkage; }

    const CFG* get_parent() const { return m_parent; }
    CFG* get_parent() { return m_parent; }
    void set_parent(CFG* parent) { m_parent = parent; }

    /// Clear the parent graph link of this function. Does not detach it.
    void clear_parent() { m_parent = nullptr; }

    /// Detach this function from its parent graph. Does not destory the
    /// function.
    void detach();

    const std::vector<FunctionArgument*>& get_args() const { return m_args; }
    std::vector<FunctionArgument*>& get_args() { return m_args; }

    /// \returns The FunctionArgument at index \p i.
    const FunctionArgument* get_arg(u32 i) const {
        assert(i <= num_args());
        return m_args[i];
    }

    FunctionArgument* get_arg(u32 i) {
        assert(i <= num_args());
        return m_args[i];
    }

    /// Mutate the argument at index \p i with \p arg.
    void set_arg(u32 i, FunctionArgument* arg) {
        assert(i <= num_args());
        m_args[i] = arg;
    }

    /// Appends a new argument \p arg to this function.
    void append_arg(FunctionArgument* arg) { m_args.push_back(arg); }

    /// \returns The number of arguments in this function.
    u32 num_args() const { return m_args.size(); }

    /// \returns `true` if this function has any arguments.
    bool has_args() const { return !m_args.empty(); }

    const std::map<std::string, Local*>& get_locals() const { return m_locals; }
    std::map<std::string, Local*>& get_locals() { return m_locals; }

    /// \returns A local in this function with the name \p name, if it exists,
    /// and `nullptr` otherwise.
    const Local* get_local(const std::string& name) const {
        auto it = m_locals.find(name);
        if (it != m_locals.end())
            return it->second;

        return nullptr;
    }

    Local* get_local(const std::string& name) {
        return const_cast<Local*>(
            static_cast<const Function*>(this)->get_local(name));
    }

    const BasicBlock* front() const { return m_front; }
    BasicBlock* front() { return m_front; }

    const BasicBlock* back() const { return m_back; }
    BasicBlock* back() { return m_back; }

    /// Prepend \p block to this function.
    void push_front(BasicBlock* block);

    /// Append \p block to this function.
    void push_back(BasicBlock* block);

    /// Insert \p block into this function at position \p idx.
    void insert(BasicBlock* block, u32 idx);

    /// Insert \p block into this function immediately after \p insert_after. 
    /// Fails if \p insert_after is not inside this function.
    void insert(BasicBlock* block, BasicBlock* insert_after);

    /// \returns `true` if this function has no basic blocks.
    bool empty() const { return m_front == nullptr; }

    /// \returns The size of this function by its basic block count.
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
};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_FUNCTION_HPP_
