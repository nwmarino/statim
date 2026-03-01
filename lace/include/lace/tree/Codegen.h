//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_CODEGEN_H_
#define LACE_CODEGEN_H_

#include "lace/tree/Stmt.h"
#include "lace/tree/VisitorBase.h"

#include "lir/graph/Builder.h"
#include "lir/graph/Function.h"
#include "lir/graph/Global.h"
#include "lir/graph/Type.h"
#include "lir/machine/Machine.h"

#include <unordered_map>
#include <vector>

namespace lace {

class Codegen final : public VisitorBase {
    enum ValueContext : uint32_t {
        Addressed,
        Valued,
    };

    struct FunctionInfo final {
        lir::Function* func = nullptr;
        bool complete = false;
    };

    struct GlobalInfo final {
        lir::Global* global = nullptr;
        bool complete = false;
    };

    lir::CFG& m_graph;
    lir::Machine& m_mach;
    lir::Builder m_builder;

    ValueContext m_vc = Valued;
    lir::Value* m_temp = nullptr;
    lir::Value* m_place = nullptr;
    lir::Function* m_func = nullptr;
    lir::BasicBlock* m_cond = nullptr;
    lir::BasicBlock* m_merge = nullptr;

    std::unordered_map<const StructType*, lir::StructType*> m_structs = {};
    std::unordered_map<const VariableDefn*, GlobalInfo> m_globals = {};
    std::unordered_map<const FunctionDefn*, FunctionInfo> m_funcs = {};

public:
    Codegen(Context& context, lir::CFG& graph, lir::Machine& mach);

    ~Codegen() = default;

    Codegen(const Codegen&) = delete;
    void operator=(const Codegen&) = delete;

    Codegen(Codegen&&) noexcept = delete;
    void operator=(Codegen&&) noexcept = delete;

    void visit(FunctionDefn& node) override;

    void visit(VariableDefn& node) override;

    void visit(AdapterStmt& node) override;

    void visit(IfStmt& node) override;

    void visit(RestartStmt& node) override;

    void visit(RetStmt& node) override;

    void visit(RuneStmt& node) override;

    void visit(StopStmt& node) override;

    void visit(UntilStmt& node) override;

    void visit(BoolLiteral& node) override;
    
    void visit(IntegerLiteral& node) override;

    void visit(CharLiteral& node) override;

    void visit(FloatLiteral& node) override;

    void visit(NullLiteral& node) override;

    void visit(StringLiteral& node) override;

    void visit(BinaryOp& node) override;

    void visit(UnaryOp& node) override;

    void visit(AccessExpr& node) override;

    void visit(CallExpr& node) override;

    void visit(CastExpr& node) override;

    void visit(ParenExpr& node) override;

    void visit(RefExpr& node) override;

    void visit(SizeofExpr& node) override;

    void visit(StructInitExpr& node) override;

    void visit(SubscriptExpr& node) override;

private:
    /// Returns the mangled name of the given |defn| under the current rib.
    std::string mangle(NamedDefn* defn);

    /// Fetch the LIR function equivelant of the given function |defn|.
    /// If |defn| has not been defined yet, it will be lowered to a definition
    /// in the IR, without a full body.
    lir::Function* fetch(FunctionDefn* defn);

    /// Fetch the LIR global equivelant of the given global variable |defn|.
    /// If |defn| has not been defined yet, it will be lowered to an empty 
    /// global definition in the IR.
    lir::Global* fetch(VariableDefn* defn);

    /// Fetch the LIR structure type equivelant of the given structure |defn|.
    /// If |defn| has not been defined yet, it will be lowered to a fully
    /// generated type in the IR.
    lir::StructType* fetch(StructDefn* defn);

    /// Fetch the LIR type equivelant of the given |type|.
    lir::Type* fetch(Type* type);

    /// Attempt to inject a boolean comparison unto the given |value|, such
    /// that the result of this function is some form of comparison with a
    /// boolean type.
    lir::Value* inject_comparison(lir::Value* value);

    /// Get or create a function in the IR with the given |name|, |result| type,
    /// and argument list |args|.
    lir::Function* get_or_create_function(const std::string& name, 
                                          lir::Type* result = nullptr,
                                          const std::vector<lir::Type*>& args = {});

    /// Returns the runtime `__copy` function.
    lir::Function* get_rtf_copy();

    /// Lower the given function |defn| to an empty IR function.
    void lower(FunctionDefn* defn);

    /// Lower the given global variable |defn| to an empty IR global.
    void lower(VariableDefn* defn);

    /// Lower the given structure |defn| to a non-empty, type-resolved 
    /// structure.
    void lower(StructDefn* defn);

    void codegen_assignment(BinaryOp& node);

    void codegen_addition(BinaryOp& node);

    void codegen_multiply(BinaryOp& node);

    void codegen_division(BinaryOp& node);

    void codegen_bitwise_arithmetic(BinaryOp& node);

    void codegen_bitwise_shift(BinaryOp& node);

    void codegen_comparison(BinaryOp& node);

    void codegen_logical_and(BinaryOp& node);

    void codegen_logical_or(BinaryOp& node);

    void codegen_negation(UnaryOp& node);

    void codegen_bitwise_not(UnaryOp& node);

    void codegen_logical_not(UnaryOp& node);

    void codegen_address_of(UnaryOp& node);

    void codegen_dereference(UnaryOp& node);
};

} // namespace lace

#endif // LACE_CODEGEN_H_
