#ifndef STATIM_TREE_ROOT_HPP_
#define STATIM_TREE_ROOT_HPP_

#include "tree/type.hpp"
#include "tree/visitor.hpp"

#include <string>
#include <unordered_map>

namespace stm {

class TypeContext final {
    friend class Root;
    friend class DeferredType;
    friend class FunctionType;
    friend class PointerType;
    friend class StructType;
    friend class EnumType;

    std::unordered_map<std::string, const Type*> types {};
    std::unordered_map<BuiltinType::Kind, BuiltinType*> builtins {};
    std::unordered_map<const Type*, PointerType*> pointers {};
    std::vector<DeferredType*> deferred {};
    std::vector<FunctionType*> functions {};
    std::vector<StructType*> structs {};
    std::vector<EnumType*> enums {};

    const Type* get(const std::string& name) const;
    const BuiltinType* get(BuiltinType::Kind kind) const;
    const PointerType* get(const Type* pPointee);
    const DeferredType* get(const DeferredType::Context& context);
    const FunctionType* get(const Type* pReturn, const std::vector<const Type*> &params);

    const StructType* create(const std::vector<const Type*>& fields, const StructDecl* decl);
    const EnumType* create(const Type* underlying, const EnumDecl* decl);

public:
    TypeContext();
    ~TypeContext();

    TypeContext(const TypeContext&) = delete;
    TypeContext& operator = (const TypeContext&) = delete;
};

/// The entire syntax tree for a single translation unit.
class Root final {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    InputFile& m_file;
    TypeContext m_context = {};
    Scope* m_scope;
    std::vector<Decl*> m_decls = {};
    std::vector<Decl*> m_imports = {};
    std::vector<Decl*> m_exports = {};

public:
    /// Create a new root that represents |file| with the given scope.
    Root(InputFile& file, Scope* scope);
    
    Root(const Root&) = delete;
    Root& operator = (const Root&) = delete;

    ~Root();

    /// Returns the input file that this tree represents.
    const InputFile& file() const { return m_file; }
    InputFile& file() { return m_file; }

    /// Returns the context used for typing in this tree.
    const TypeContext& context() const { return m_context; }
    TypeContext& context() { return m_context; }

    /// Returns the global scope of this tree.
    const Scope* get_scope() const { return m_scope; }
    Scope* get_scope() { return m_scope; }

    /// Returns all the top-level declarations in this tree.
    const std::vector<Decl*>& decls() const { return m_decls; }
    std::vector<Decl*>& decls() { return m_decls; }

    /// Returns the number of top-level declarations in this tree.
    u32 num_decls() const { return decls().size(); }

    /// Add |decl| to the list of declarations in this tree.
    void add_decl(Decl* decl) { decls().push_back(decl); }

    /// Returns all the use declarations in this tree.
    std::vector<UseDecl*> uses() const;

    /// Returns all the declaration that this tree impors.
    const std::vector<Decl*>& imports() const { return m_imports; }
    std::vector<Decl*>& imports() { return m_imports; }

    /// Returns all the declaration that this tree can export.
    const std::vector<Decl*>& exports() const { return m_exports; }
    std::vector<Decl*>& exports() { return m_exports; }

    const BuiltinType* get_void_type() const { 
        return m_context.get(BuiltinType::Kind::Void); 
    }
    
    const BuiltinType* get_bool_type() const { 
        return m_context.get(BuiltinType::Kind::Bool); 
    }

    const BuiltinType* get_char_type() const { 
        return m_context.get(BuiltinType::Kind::Char); 
    }

    const BuiltinType* get_si8_type() const { 
        return m_context.get(BuiltinType::Kind::SInt8); 
    }

    const BuiltinType* get_si16_type() const { 
        return m_context.get(BuiltinType::Kind::SInt16); 
    }

    const BuiltinType* get_si32_type() const { 
        return m_context.get(BuiltinType::Kind::SInt32); 
    }

    const BuiltinType* get_si64_type() const { 
        return m_context.get(BuiltinType::Kind::SInt64); 
    }

    const BuiltinType* get_ui8_type() const { 
        return m_context.get(BuiltinType::Kind::UInt8); 
    }

    const BuiltinType* get_ui16_type() const { 
        return m_context.get(BuiltinType::Kind::UInt16); 
    }

    const BuiltinType* get_ui32_type() const { 
        return m_context.get(BuiltinType::Kind::UInt32); 
    }

    const BuiltinType* get_ui64_type() const { 
        return m_context.get(BuiltinType::Kind::UInt64);
    }

    const BuiltinType* get_fp32_type() const { 
        return m_context.get(BuiltinType::Kind::Float32);
    }

    const BuiltinType* get_fp64_type() const { 
        return m_context.get(BuiltinType::Kind::Float64); 
    }

    /// Validate this AST, preparing it for semantic analysis passes.
    ///
    /// This function mainly resolves all deferred types within its context,
    /// and does some small adjustments to make passes over the tree simpler.s
    void validate();

    void accept(Visitor& visitor) {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const;
};

} // namespace stm

#endif // STATIM_TREE_ROOT_HPP_
