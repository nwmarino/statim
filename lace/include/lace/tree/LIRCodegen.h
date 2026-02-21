//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_LIR_CODEGEN_H_
#define LACE_LIR_CODEGEN_H_

//
//  This header file declares the LIRCodegen class, whom instances thereof 
//  generate LIR code from a valid abstract syntax tree.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Stmt.h"
#include "lace/tree/Type.h"
#include "lace/tree/VisitorBase.h"

#include "lir/graph/BasicBlock.h"
#include "lir/graph/Builder.h"
#include "lir/graph/CFG.h"
#include "lir/graph/Function.h"
#include "lir/graph/Type.h"
#include "lir/machine/Machine.h"

namespace lace {

class LIRCodegen final {
    struct State final {
        lir::BasicBlock* cond = nullptr;
        lir::BasicBlock* merge = nullptr;
        lir::Value* place = nullptr;
    };

    const Options& m_options;
    const lir::Machine& m_mach;

    const AST* m_ast;
    lir::CFG& m_cfg;
    lir::Builder m_builder;
    lir::Function* m_func = nullptr;
    State m_state = {};

public:
    LIRCodegen(const Options& options, const AST* ast, lir::CFG& cfg)
      : m_options(options), m_mach(cfg.get_machine()), m_ast(ast), m_cfg(cfg),
        m_builder(cfg) {}

    LIRCodegen(const LIRCodegen&) = delete;
    void operator=(const LIRCodegen&) = delete;

    LIRCodegen(LIRCodegen&&) noexcept = delete;
    void operator=(LIRCodegen&&) noexcept = delete;

    /// Run the code generation process.
    void run();

private:
    /// Lower the given lace |type| to its LIR equivelant, where possible.
    lir::Type* to_lir_type(const Type* type);

    lir::Function* get_function(const std::string& name, lir::Type* result = nullptr,
                                const std::vector<lir::Type*>& args = {});

    lir::Function* getIntrinsicCopy();

    /// Attempt to inject a boolean comparison unto the given |value|, such
    /// that the result is some form of comparison of a boolean type.
    ///
    /// Ultimately, the type of the returned value will be a 8-bit integer 
    /// representation a.k.a boolean.
    lir::Value* inject_comparison(lir::Value* value);

    /// Generate an empty lowering for the given |defn|.
    void codegen_initial_definition(const Defn* defn);

    /// Generate code for the body of the given |defn|. 
    /// Assumes that the definition has been lowered already, and exists by 
    /// name in the graph.
    void codegen_lowered_definition(const Defn* defn);

    lir::Function* codegen_initial_function(const FunctionDefn* defn);
    lir::Function* codegen_lowered_function(const FunctionDefn* defn);

    lir::Global* codegen_initial_global(const VariableDefn* defn);
    lir::Global* codegen_lowered_global(const VariableDefn* defn);

    lir::StructType* codegen_initial_structure(const StructDefn* defn);
    lir::StructType* codegen_lowered_structure(const StructDefn* defn);

    /// Generate a LIR local for the given local variable |defn|.
    lir::Local* codegen_local_variable(const VariableDefn* defn);

    /// Generate a value (rvalue) for the given |expr|.
    lir::Value* codegen_valued_expression(const Expr* expr);

    /// Generate an address (lvalue) for the given |expr|.
    lir::Value* codegen_addressed_expression(const Expr* expr);

    lir::Value* codegen_addressed_access(const AccessExpr* expr);
    lir::Value* codegen_valued_access(const AccessExpr* expr);

    lir::Value* codegen_addressed_reference(const RefExpr* expr);
    lir::Value* codegen_valued_reference(const RefExpr* expr);

    lir::Value* codegen_addressed_subscript(const SubscriptExpr* expr);
    lir::Value* codegen_valued_subscript(const SubscriptExpr* expr);

    lir::Value* codegen_addressed_dereference(const UnaryOp* expr);
    lir::Value* codegen_valued_dereference(const UnaryOp* expr);

    /// Generate an assignment '=' expression. Returns the valued right operand.
    lir::Value* codegen_assignment(const BinaryOp* expr);

    /// Generate an add '+' or subtract '-' expression. Returns the resulting 
    /// value.
    lir::Value* codegen_addition(const BinaryOp* expr);

    /// Generate a multiply '*' expression. Returns the resulting value.
    lir::Value* codegen_multiply(const BinaryOp* expr);

    /// Generate a division '/' or modulo '%' expression. Returns the resulting 
    /// value.
    lir::Value* codegen_division(const BinaryOp* expr);

    /// Generate a bitwise arithmetic expression, in particular one of the
    /// '&', '|', '^' operators. Returns the resulting value.
    lir::Value* codegen_bitwise_arithmetic(const BinaryOp* expr);

    /// Generate a bit shift '<<', '>>' expression. Returns the resulting value.
    lir::Value* codegen_bit_shift(const BinaryOp* expr);

    /// Generate a numerical comparison expression. Returns the resulting 
    /// boolean value.
    lir::Value* codegen_numerical_comparison(const BinaryOp* expr);

    /// Generate a logical and '&&' expression. Returns the value that is the 
    /// result of the control flow.
    lir::Value* codegen_logical_and(const BinaryOp* expr);

    /// Generate a logical or '||' expression. Returns the value that is the 
    /// result of the control flow.
    lir::Value* codegen_logical_or(const BinaryOp* expr);

    /// Generate a negation '-' expression. Returns the resulting value.
    lir::Value* codegen_negation(const UnaryOp* expr);

    /// Generate a bitwise not '~' expression. Returns the resulting value.
    lir::Value* codegen_bitwise_not(const UnaryOp* expr);

    /// Generate a logical not '!' expression. Returns the resulting value.
    lir::Value* codegen_logical_not(const UnaryOp* expr);

    /// Generate an address-of '&' expression. Returns the resulting value.
    lir::Value* codegen_address_of(const UnaryOp* expr);

    lir::Value* codegen_literal_boolean(const BoolLiteral* expr);
    lir::Value* codegen_literal_integer(const IntegerLiteral* expr);
    lir::Value* codegen_literal_character(const CharLiteral* expr);
    lir::Value* codegen_literal_float(const FloatLiteral* expr);
    lir::Value* codegen_literal_null(const NullLiteral* expr);
    lir::Value* codegen_literal_string(const StringLiteral* expr);

    lir::Value* codegen_type_cast(const CastExpr* expr);
    lir::Value* codegen_function_call(const CallExpr* expr);
    lir::Value* codegen_parentheses(const ParenExpr* expr);
    lir::Value* codegen_sizeof(const SizeofExpr* expr);
    lir::Value* codegen_struct_init(const StructInitExpr* expr);

    /// Generate code for an arbitrary |stmt|.
    void codegen_statement(const Stmt *stmt);
    
    void codegen_adapter(const AdapterStmt* stmt);
    void codegen_block(const BlockStmt* stmt);
    
    void codegen_if(const IfStmt* stmt);
    void codegen_until(const UntilStmt* stmt);

    void codegen_restart(const RestartStmt* stmt);
    void codegen_stop(const StopStmt* stmt);
    void codegen_return(const RetStmt* stmt);

    void codegen_rune_statement(const RuneStmt* stmt);
};

} // namespace lace

#endif // LACE_LIR_CODEGEN_H_
