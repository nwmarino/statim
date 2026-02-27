//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_RIB_H_
#define LACE_RIB_H_

//
//  This header file declares the Rib type, which represents a set of files 
//  in a source program, and their top-level symbols.
//

#include "lace/tree/Type.h"
#include "lace/tree/VisitorBase.h"

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace lace {

class Defn;
class Scope;

class Rib final {
    friend class AliasType;
    friend class BuiltinType;
    friend class DeferredType;
    friend class EnumType;
    friend class FunctionType;
    friend class PointerType;
    friend class StructType;

    using AliasTypePool = std::unordered_map<std::string, AliasType*>;
    using BuiltinTypePool = std::vector<BuiltinType*>;
    using DeferredTypePool = std::vector<DeferredType*>;
    using EnumTypePool = std::unordered_map<std::string, EnumType*>;
    using FunctionTypePool = std::vector<FunctionType*>;
    using PointerTypePool = std::vector<PointerType*>;
    using StructTypePool = std::unordered_map<std::string, StructType*>;

    /// The name of this rib, e.g. `stl::io`.
    std::string m_name;

    /// The path of the original file which defined this rib.
    std::string m_path;

    /// The global scope of this rib.
    Scope* m_scope;

    /// The top-level definitions of this rib.
    std::vector<Defn*> m_defns = {};

    struct {
        AliasTypePool aliases = {};
        BuiltinTypePool builtins = {};
        DeferredTypePool deferred = {};
        EnumTypePool enums = {};
        FunctionTypePool functions = {};
        PointerTypePool pointers = {};
        StructTypePool structs = {};
    } m_types;

    Rib(const std::string& name, const std::string& path);

public:
    [[nodiscard]] 
    static Rib* create(const std::string& name, const std::string& path);

    ~Rib();

    Rib(const Rib&) = delete;
    void operator=(const Rib&) = delete;

    Rib(Rib&&) noexcept = delete;
    void operator=(Rib&&) noexcept = delete;

    void accept(VisitorBase& visitor) { visitor.visit(*this); }

    /// Returns the name of this rib.
    const std::string& name() const { return m_name; }

    /// Returns the path of the file which originally defined this rib.
    const std::string& path() const { return m_path; }

    /// Returns the scope of this rib.
    const Scope* scope() const { return m_scope; }
    Scope* scope() { return m_scope; }

    /// Returns the definition list of this rib.
    const std::vector<Defn*>& defns() const { return m_defns; }
    std::vector<Defn*>& defns() { return m_defns; }

    /// Returns the |i|-th definition in this rib.
    const Defn* get_defn(uint32_t i) const {
        assert(i <= m_defns.size() && "index out of bounds!");
        return m_defns[i];
    }

    Defn* get_defn(uint32_t i) {
        assert(i <= m_defns.size() && "index out of bounds!");
        return m_defns[i];
    }

    /// Returns the number of definitions in this rib.
    uint32_t num_defns() const { return m_defns.size(); }

    /// Test if this rib has any definitions.
    bool has_defns() const { return !m_defns.empty(); }

    /// Test if this rib is empty i.e. contains no definitions.
    [[nodiscard]] bool empty() const { return m_defns.empty(); }
};

} // namespace lace

#endif // LACE_RIB_H_
