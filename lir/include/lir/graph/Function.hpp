//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_FUNCTION_H_
#define LOVELACE_IR_FUNCTION_H_

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Local.hpp"
#include "lir/graph/Parameter.hpp"
#include "lir/graph/Type.hpp"
#include "lir/graph/Value.hpp"

#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace lir {

class CFG;
class Parameter;

/// A function consisting of basic blocks.
class Function final : public Value {
public:
    using Params = std::vector<Parameter*>;
    using Locals = std::map<std::string, Local*>;

    /// The different kinds of linkage a function can have.
    enum class LinkageType : uint8_t {
        Public,
        Private,
    };

private:
    CFG *m_parent;
    LinkageType m_linkage;
    std::string m_name;
    Params m_params = {};
    Locals m_locals = {};
    BasicBlock *m_head = nullptr;
    BasicBlock *m_tail = nullptr;

    Function(FunctionType *type, CFG *parent, LinkageType linkage, 
             const std::string &name, const Params &params)
      : Value(type), m_parent(parent), m_linkage(linkage), m_name(name), 
        m_params(params) {}

public:
    [[nodiscard]] static 
    Function *create(CFG &cfg, LinkageType linkage, FunctionType *type, 
                     const std::string &name, const Params &params);

    ~Function() override;

    Function(const Function&) = delete;
    void operator=(const Function&) = delete;

    Function(Function&&) noexcept = delete;
    void operator=(Function&&) noexcept = delete;

    void set_linkage(LinkageType linkage) { m_linkage = linkage; }
    LinkageType get_linkage() const { return m_linkage; }

    /// Test if this function has the given |linkage| type.
    bool has_linkage(LinkageType linkage) const { 
        return m_linkage == linkage; 
    }
    
    const FunctionType *get_type() const { 
        return static_cast<const FunctionType*>(m_type); 
    }
    
    FunctionType *get_type() { 
        return static_cast<FunctionType*>(m_type); 
    }

    void set_name(const std::string &name) { m_name = name; }
    const std::string &get_name() const { return m_name; }
    std::string &get_name() { return m_name; }

    void set_parent(CFG *cfg) { m_parent = cfg; }
    const CFG *get_parent() const { return m_parent; }
    CFG *get_parent() { return m_parent; }

    /// Detach this function from its parent graph.
    ///
    /// Does not free any memory allocated for this function.
    void detach();

    const Params &get_params() const { return m_params; }
    Params &get_params() { return m_params; }

    /// Returns the number of parameters this function has.
    uint32_t num_params() const { return m_params.size(); }

    /// Test if this function has any parameters.
    bool has_params() const { return !m_params.empty(); }

    /// Returns the |i|-th parameter of this function.
    const Parameter *get_param(uint32_t i) const {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }

    Parameter *get_param(uint32_t i) {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }

    /// Returns the parameter in this function with the given |name| if it 
    /// exists, and null otherwise.
    const Parameter *get_param(const std::string &name) const {
        for (Parameter *param : m_params) {
            if (param->get_name() == name)
                return param;
        }

        return nullptr;
    }

    Parameter *get_param(const std::string &name) {
        return const_cast<Parameter*>(
            static_cast<const Function*>(this)->get_param(name));
    }

    /// Append the given |param| to the back of this functions' parameter list,
    /// and returns the result of the addition.
    ///
    /// If the parameter were to conflict name-wise with another symbol already
    /// in this function, then the call fails.
    bool add_param(Parameter *param);

    /// Remove the given |param| from this function. If it does not belong to
    /// this function, then the call fails silently.
    void remove_param(Parameter *param);

    /// Test if this function contains an aggregate return parameter.
    Result hasAggregateReturn() const {
        if (m_params.size() < 1)
            return false;

        return m_params[0]->hasTrait(Parameter::Trait::ARet);
    }

    const Locals &get_locals() const { return m_locals; }
    Locals &get_locals() { return m_locals; }

    /// Returns the number of locals in this function.
    uint32_t num_locals() const { return m_locals.size(); }

    /// Test if this function has any locals.
    bool has_locals() const { return !m_locals.empty(); }

    /// Returns the local in this function with the given |name| if one exists,
    /// and null otherwise.
    const Local *get_local(const std::string &name) const;
    Local *get_local(const std::string &name) {
        return const_cast<Local*>(
            static_cast<const Function*>(this)->get_local(name));
    }

    /// Add the given |local| to this function, and returns the result of the
    /// addition.
    ///
    /// If the local were to conflict name-wise with another symbol already in
    /// this function, then the call fails.
    bool add_local(Local *local);

    /// Remove the given |local| from this function. If it does not belong to
    /// this function, then the call fails silently.
    void remove_local(Local *local);

    void set_head(BasicBlock *block) { m_head = block; }
    const BasicBlock *get_head() const { return m_head; }
    BasicBlock *get_head() { return m_head; }

    void set_tail(BasicBlock *block) { m_tail = block; }
    const BasicBlock *get_tail() const { return m_tail; }
    BasicBlock *get_tail() { return m_tail; }

    /// Prepend the given |block| to the front of this function.
    void prepend(BasicBlock *block);

    /// Append the given |block| to the back of this function.
    void append(BasicBlock *block);

    /// Insert the given |block| into this function at position |i|.
    void insert(BasicBlock *block, uint32_t i);

    /// Insert |block| into this function immediately after |insert_after|. 
    /// Fails if |insert_after| does not already belong to this function.
    void insert(BasicBlock *block, BasicBlock *insert_after);

    /// Remove the given |block| from this function, if it belongs to it.
    void remove(BasicBlock *block);

    /// Test if this function has no basic blocks.
    bool empty() const { return m_head == nullptr; }

    /// Returns the size of this function by the number of basic blocks in it.
    uint32_t size() const;

    void print(std::ostream &os, PrintPolicy policy) const override;
};

} // namespace lir

#endif // LOVELACE_IR_FUNCTION_H_
