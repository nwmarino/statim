#ifndef STATIM_SIIR_FUNCTION_HPP_
#define STATIM_SIIR_FUNCTION_HPP_

#include "siir/basicblock.hpp"
#include "siir/local.hpp"
#include "siir/type.hpp"
#include "siir/value.hpp"

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace stm {
namespace siir {

class CFG;

/// An argument to a function.
class Argument final : public Value {
    /// The parent function of this argument.
    Function* m_parent;

    /// The name of this argument.
    std::string m_name;

    /// The position of this argument in its parent function.
    u32 m_number;

public:
    /// Create a new argument for position |number| in function |parent|. 
    Argument(const Type* type, const std::string& name, u32 number, 
             Function* parent = nullptr);

    Argument(const Argument&) = delete;
    Argument& operator = (const Argument&) = delete;

    /// Returns the parent function of this argument.
    const Function* get_parent() const { return m_parent; }
    Function* get_parent() { return m_parent; }

    /// Clear the link to the parent function of this argument.
    void clear_parent() { m_parent = nullptr; }

    /// Set the parent function of this argument to |parent|.
    void set_parent(Function* parent) { m_parent = parent; }

    /// Get the name of this argument.
    const std::string& get_name() const { return m_name; }

    /// Rename this argument to |name|.
    void rename(const std::string& name) { m_name = name; }

    /// Returns the number of this argument in its parent function.
    u32 get_number() const { return m_number; }

    /// Mutate the number of this argument to |number|.
    void set_number(u32 number) { m_number = number; }

    void print(std::ostream& os) const override;
};
 
/// A function routine consisting of basic blocks.
class Function final : public Value {
public:
    /// Recognized linkage types for global functions.
    enum LinkageType : u8 {
        LINKAGE_INTERNAL,
        LINKAGE_EXTERNAL,
    };

private:
    /// The parent graph of this function.
    CFG* m_parent;

    /// The name of this function.
    std::string m_name;

    /// The list of arguments that this function uses.
    std::vector<Argument*> m_args;

    /// The stack-based locals of this function.
    std::map<std::string, Local*> m_locals = {};

    /// Links to the first and last basic blocks of this function.
    BasicBlock* m_front = nullptr;
    BasicBlock* m_back = nullptr;

    /// The linkage type of this function.
    LinkageType m_linkage;

public:
    /// Create a new function. Providing |parent| does not automatically add
    /// the new function to the given graph.
    Function(CFG& cfg, LinkageType linkage, const FunctionType* type, 
             const std::string& name, const std::vector<Argument*>& args);

    Function(const Function&) = delete;
    Function& operator = (const Function&) = delete;

    ~Function() override;

    /// Get the linkage type of this function.
    LinkageType get_linkage() const { return m_linkage; }
    
    /// Mutate the linkage type of this function to |linkage|.
    void set_linkage(LinkageType linkage) { m_linkage = linkage; }

    /// Get the type of this function.
    const FunctionType* get_type() const { 
        return static_cast<const FunctionType*>(m_type); 
    }

    /// Get the return type of this function.
    const Type* get_return_type() const {
        return static_cast<const FunctionType*>(m_type)->get_return_type();
    }

    /// Get the name of this function.
    const std::string& get_name() const { return m_name; }

    /// Rename this function to |name|.
    void rename(const std::string& name) { m_name = name; }

    /// Get the parent graph of this function.
    const CFG* get_parent() const { return m_parent; }
    CFG* get_parent() { return m_parent; }

    /// Clear the parent graph link of this function. Does not detach the
    /// function.
    void clear_parent() { m_parent = nullptr; }

    /// Mutate the parent graph of this function to |parent|. Does not add
    /// this function to the new graph, nor remove it from an existing parent.
    void set_parent(CFG* parent) { m_parent = parent; }

    /// Detach this function from its parent graph. Does not destory the
    /// function.
    void detach_from_parent();

    /// Returns the arguments in this function.
    const std::vector<Argument*>& args() const { return m_args; }
    std::vector<Argument*>& args() { return m_args; }

    /// Returns the number of arguments in this function.
    u32 num_args() const { return m_args.size(); }

    /// Returns true if this function has atleast one argument.
    bool has_args() const { return !m_args.empty(); }

    /// Returns the argument at index |i| if it exists.
    const Argument* get_arg(u32 i) const;
    Argument* get_arg(u32 i) {
        return const_cast<Argument*>(
            static_cast<const Function*>(this)->get_arg(i));
    }

    /// Mutate the argument at position |i| with |arg|.
    void set_arg(u32 i, Argument* arg);

    /// Append |arg| to this functions' argument list.
    void append_arg(Argument* arg) { m_args.push_back(arg); }

    /// Returns the locals in this function.
    const std::map<std::string, Local*>& locals() const { return m_locals; }
    std::map<std::string, Local*>& locals() { return m_locals; }

    /// Returns the local in this function by name if one exists.
    const Local* get_local(const std::string& name) const;
    Local* get_local(const std::string& name) {
        return const_cast<Local*>(
            static_cast<const Function*>(this)->get_local(name));
    }

    /// Add |local| to this functions list of locals. Fails if there is
    /// already an existing local with the same name.
    void add_local(Local* local);

    /// Remove |local| from this function if it already belongs.
    void remove_local(Local* local);

    /// Returns the first basic block of this function, if one exists.
    const BasicBlock* front() const { return m_front; }
    BasicBlock* front() { return m_front; }

    /// Returns the last basic block of this function, if one exists.
    const BasicBlock* back() const { return m_back; }
    BasicBlock* back() { return m_back; }

    /// Prepend |blk| to this function.
    void push_front(BasicBlock* blk);

    /// Append |blk| to this function.
    void push_back(BasicBlock* blk);

    /// Insert |blk| into this function at position |i|.
    void insert(BasicBlock* blk, u32 i);

    /// Insert |blk| into this function immediately after |insert_after|. 
    /// Fails if |insert_after| does not already belong to this function.
    void insert(BasicBlock* blk, BasicBlock* insert_after);

    /// Remove the basic block |blk| if it belongs to this function.
    void remove(BasicBlock* blk);

    /// Returns true if this function has no basic blocks.
    bool empty() const { return m_front == nullptr; }

    /// Returns the size of this function by the number of basic blocks in it.
    u32 size() const { return std::distance(begin(), end()); }

    void print(std::ostream& os) const override;

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
