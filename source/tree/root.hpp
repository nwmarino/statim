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

class Root final {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    InputFile&                  file;
    TypeContext                 context;
    Scope*                      pScope;
    std::vector<Decl*>          decls;
    std::vector<Decl*>    imports;
    std::vector<Decl*>    exports;

public:
    Root(InputFile& file, Scope* pScope);
    
    ~Root();

    InputFile& get_file() { return file; }

    TypeContext& get_context() { return context; }

    Scope* get_scope() { return pScope; }

    const Scope* get_scope() const { return pScope; }

    const std::vector<Decl*>& get_decls() const { return decls; }

    u32 num_decls() const { return decls.size(); }

    void add_decl(Decl* pDecl) { decls.push_back(pDecl); }

    const std::vector<Decl*>& get_imports() const { return imports; }

    const std::vector<Decl*>& get_exports() const { return exports; }

    const BuiltinType* get_void_type() const 
    { return context.get(BuiltinType::Kind::Void); }
    
    const BuiltinType* get_bool_type() const 
    { return context.get(BuiltinType::Kind::Bool); }

    const BuiltinType* get_char_type() const 
    { return context.get(BuiltinType::Kind::Char); }

    const BuiltinType* get_si8_type() const 
    { return context.get(BuiltinType::Kind::SInt8); }

    const BuiltinType* get_si16_type() const 
    { return context.get(BuiltinType::Kind::SInt16); }

    const BuiltinType* get_si32_type() const 
    { return context.get(BuiltinType::Kind::SInt32); }

    const BuiltinType* get_si64_type() const 
    { return context.get(BuiltinType::Kind::SInt64); }

    const BuiltinType* get_ui8_type() const 
    { return context.get(BuiltinType::Kind::UInt8); }

    const BuiltinType* get_ui16_type() const 
    { return context.get(BuiltinType::Kind::UInt16); }

    const BuiltinType* get_ui32_type() const 
    { return context.get(BuiltinType::Kind::UInt32); }

    const BuiltinType* get_ui64_type() const 
    { return context.get(BuiltinType::Kind::UInt64); }

    const BuiltinType* get_fp32_type() const 
    { return context.get(BuiltinType::Kind::Float32); }

    const BuiltinType* get_fp64_type() const 
    { return context.get(BuiltinType::Kind::Float64); }

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
