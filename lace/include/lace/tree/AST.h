//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_AST_H_
#define LOVELACE_AST_H_

//
//  This header file declares the AST type, which represents the root of an
//  abstract syntax tree parsed from a source file.
//
//  It also includes the nested Context class, which is used as a manager for
//  frontend type ownership.
//

#include "lace/tree/VisitorBase.h"

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace lace {

class AliasType;
class ArrayType;
class BuiltinType;
class DeferredType;
class Defn;
class EnumType;
class FunctionType;
class PointerType;
class Scope;
class StructType;

class AST final {
public:
    using Defns = std::vector<Defn*>;

    class Context final {
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

        AliasTypePool m_aliases = {};
        BuiltinTypePool m_builtins = {};
        DeferredTypePool m_deferred = {};
        EnumTypePool m_enums = {};
        FunctionTypePool m_functions = {};
        PointerTypePool m_pointers = {};
        StructTypePool m_structs = {};

    public:
        Context();

        ~Context();

        Context(const Context&) = delete;
        void operator=(const Context&) = delete;

        Context(Context&&) noexcept = delete;
        void operator=(Context&&) noexcept = delete;
    };

private:
    Context m_context = {};
    std::string m_file;
    std::vector<Defn*> m_defns = {};
    std::vector<Defn*> m_loaded = {};
    Scope* m_scope = nullptr;

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

    void accept(VisitorBase& visitor) { 
        visitor.visit(*this); 
    }

    const std::string& get_file() const { return m_file; }

    const Context& get_context() const { return m_context; }
    Context& get_context() { return m_context; }

    const Defns& get_defns() const { return m_defns; }
    Defns& get_defns() { return m_defns; }

    const Defn* get_defn(uint32_t i) const {
        assert(i <= num_defns() && "index out of bounds!");
        return m_defns[i];
    }

    Defn* get_defn(uint32_t i) {
        assert(i <= num_defns() && "index out of bounds!");
        return m_defns[i];
    }

    uint32_t num_defns() const { return m_defns.size(); }
    bool has_defns() const { return !m_defns.empty(); }

    const Defns& get_loaded() const { return m_loaded; }
    Defns& get_loaded() { return m_loaded; }

    uint32_t num_loaded() const { return m_loaded.size(); }

    const Scope* get_scope() const { return m_scope; }
    Scope* get_scope() { return m_scope; }
};

} // namespace lace

#endif // LOVELACE_AST_H_
