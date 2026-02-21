//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_AST_H_
#define LACE_AST_H_

//
//  This header file declares the AST type, which represents the root of an
//  abstract syntax tree parsed from a source file.
//
//  It also includes the nested Context class, which is used as a manager for
//  frontend type ownership.
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

class AST final {
    friend class SymbolAnalysis;
    friend class AliasType;
    friend class ArrayType;
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

    std::string m_file;
    std::vector<Defn*> m_defns = {};
    std::vector<Defn*> m_imports = {};
    Scope* m_scope = nullptr;

    struct {
        AliasTypePool aliases = {};
        BuiltinTypePool builtins = {};
        DeferredTypePool deferred = {};
        EnumTypePool enums = {};
        FunctionTypePool functions = {};
        PointerTypePool pointers = {};
        StructTypePool structs = {};
    } m_types;

    AST(const std::string& file);

public:
    /// Create a new abstract syntax tree representing the given |file|.
    [[nodiscard]] 
    static AST* create(const std::string& file);

    ~AST();

    AST(const AST&) = delete;
    void operator=(const AST&) = delete;

    AST(AST&&) noexcept = delete;
    void operator=(AST&&) noexcept = delete;

    void accept(VisitorBase& visitor) { visitor.visit(*this); }

    /// Returns the path of the file which this syntax tree represents.
    const std::string& get_file() const { return m_file; }

    /// Returns the definitions which are defined in the file represented by
    /// this syntax tree.
    const std::vector<Defn*>& defns() const { return m_defns; }
    std::vector<Defn*>& defns() { return m_defns; }

    /// Returns the |i|-th definition in this syntax tree.
    const Defn* get_defn(uint32_t i) const {
        assert(i <= m_defns.size() && "index out of bounds!");
        return m_defns[i];
    }

    Defn* get_defn(uint32_t i) {
        assert(i <= m_defns.size() && "index out of bounds!");
        return m_defns[i];
    }

    /// Returns the number of definitions in this syntax tree.
    uint32_t num_defns() const { return m_defns.size(); }

    /// Test if this syntax tree has any definitions.
    bool has_defns() const { return !m_defns.empty(); }

    /// Returns the definition which this syntax tree imports.
    const std::vector<Defn*>& imports() const { return m_imports; }
    std::vector<Defn*>& imports() { return m_imports; }

    /// Returns the number of definitions which this syntax tree imports.
    uint32_t num_imports() const { return m_imports.size(); }

    /// Test if this syntax tree imports any definitions.
    bool has_imports() const { return !m_imports.empty(); }

    /// Returns the global scope of this syntax tree.
    const Scope* scope() const { return m_scope; }
    Scope* scope() { return m_scope; }
};

} // namespace lace

#endif // LACE_AST_H_
