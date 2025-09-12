#include "core/logger.hpp"
#include "tree/type.hpp"
#include "tree/decl.hpp"
#include "tree/expr.hpp"
#include "tree/root.hpp"
#include "tree/rune.hpp"
#include "tree/scope.hpp"
#include "tree/stmt.hpp"
#include "tree/visitor.hpp"

using namespace stm;

SymbolAnalysis::SymbolAnalysis(Options& opts, Root& root) 
    : opts(opts), root(root), pScope(root.get_scope()) {};

void SymbolAnalysis::visit(Root& node) {
    for (auto& decl : node.decls()) 
        decl->accept(*this);
}

void SymbolAnalysis::visit(FunctionDecl& node) {
    pScope = node.get_scope();
    if (node.has_body()) node.pBody->accept(*this);
    pScope = pScope->get_parent();
}

void SymbolAnalysis::visit(VariableDecl& node) {
    if (node.has_init()) 
        node.get_init()->accept(*this);

    if (node.get_type() != nullptr)
        return;

    // If the variable has no type specifier, it needs an initializer in order
    // to infer it.
    if (!node.has_init()) {
        Logger::fatal(
            "cannot infer variable type without initializer", 
            node.get_span());
    }

    node.m_type = node.get_init()->get_type();
}

void SymbolAnalysis::visit(AsmStmt& node) {
    for (auto& expr : node.m_exprs)
        expr->accept(*this);
}

void SymbolAnalysis::visit(BlockStmt& node) {
    pScope = node.get_scope();
    for (auto stmt : node.get_stmts()) stmt->accept(*this);
    pScope = pScope->get_parent();
}

void SymbolAnalysis::visit(DeclStmt& node) {
    node.pDecl->accept(*this);
}

void SymbolAnalysis::visit(IfStmt& node) {
    node.pCond->accept(*this);
    node.pThen->accept(*this);

    if (node.has_else()) node.pElse->accept(*this);
}

void SymbolAnalysis::visit(WhileStmt& node) {
    node.pCond->accept(*this);
    node.pBody->accept(*this);
}

void SymbolAnalysis::visit(RetStmt& node) {
    if (node.has_expr()) node.pExpr->accept(*this);
}

void SymbolAnalysis::visit(BinaryExpr& node) {
    node.pLeft->accept(*this);
    node.pRight->accept(*this);
    node.pType = node.get_lhs()->get_type();
}

void SymbolAnalysis::visit(UnaryExpr& node) {
    node.pExpr->accept(*this);
    node.pType = node.get_expr()->get_type();

    switch (node.get_operator()) {
    case UnaryExpr::Operator::Dereference:
        if (auto ptr = dynamic_cast<const PointerType*>(
                node.get_expr()->get_type())) {
            node.pType = ptr->get_pointee();
        } else {
            Logger::fatal("cannot apply '*' operator to non-pointer", node.span);
        }

        break;

    case UnaryExpr::Operator::Address_Of:
        if (!node.get_expr()->is_lvalue())
            Logger::fatal("cannot apply '&' operator to non-lvalue", node.span);
        
        node.pType = PointerType::get(root, node.get_expr()->get_type());
        break;

    case UnaryExpr::Operator::Logical_Not:
        node.pExpr->pType = BuiltinType::get(root, BuiltinType::Kind::Bool);
        break;

    default:
        break;
    }
}

void SymbolAnalysis::visit(CastExpr& node) {
    node.pExpr->accept(*this);
}

void SymbolAnalysis::visit(ParenExpr& node) {
    node.pExpr->accept(*this);
    node.pType = node.get_expr()->get_type();
}

void SymbolAnalysis::visit(SubscriptExpr& node) {
    node.pBase->accept(*this);
    node.pIndex->accept(*this);

    const Type* base_type = node.get_base()->get_type();
    if (!base_type->is_pointer()) {
        Logger::fatal(
            "operator '[]' base type must be a pointer, got '" + 
                base_type->to_string() + "'", 
            node.get_span());
    }

    node.pType = base_type->as_pointer()->get_pointee();

    if (!node.get_index()->get_type()->is_int()) {
        Logger::fatal(
            "subscript operator '[]' index type must be an integer", 
            node.get_span());
    }
}

void SymbolAnalysis::visit(ReferenceExpr& node) {
    Decl* decl = pScope->get(node.get_name());
    if (!decl)
        Logger::fatal("unresolved reference: '" + node.name + "'", node.span);

    if (auto var = dynamic_cast<VariableDecl*>(decl)) {
        node.pType = var->get_type();
    } else if (auto param = dynamic_cast<ParameterDecl*>(decl)) {
        node.pType = param->get_type();
    } else if (auto val = dynamic_cast<EnumValueDecl*>(decl)) {
        node.pType = val->get_type();
    } else {
        Logger::fatal("unresolved reference: '" + node.name + "'", node.span);
    }

    node.set_decl(decl);
}

void SymbolAnalysis::visit(MemberExpr& node) {
    node.pBase->accept(*this);

    // Try to resolve the type of the access base as a structure.
    const StructType* st = nullptr;
    const Type* base_type = node.get_base()->get_type();
    if (base_type->is_struct()) {
        st = base_type->as_struct();
    } else if (base_type->is_pointer()) {
        auto pointee = base_type->as_pointer()->get_pointee();
        st = dynamic_cast<const StructType*>(pointee);
        if (!st) {
            Logger::fatal(
                "access operator '.' base is a pointer, but not to a struct",
                node.get_span());
        }
    } else {
        Logger::fatal(
            "access operator '.' base is not a structure",
            node.get_span());
    }

    // Try to resolve the targetted field within the structure.
    const StructDecl* decl = st->get_decl();
    const FieldDecl* field = decl->get_field(node.get_name());
    if (!field) {
        Logger::fatal(
            "member '" + node.get_name() + "' does not exist in struct '" + 
                decl->get_name() + "'",
            node.get_span());
    }

    node.set_decl(field);
    node.pType = field->get_type();
}

void SymbolAnalysis::visit(CallExpr& node) {
    for (auto arg : node.args)
        arg->accept(*this);

    // Try to resolve the callee and ensure it's a function.
    auto decl = pScope->get(node.get_name());
    if (!decl) {
        Logger::fatal(
            "unresolved reference: '" + node.get_name() + "'", 
            node.get_span());
    }

    auto function = dynamic_cast<FunctionDecl*>(decl);
    if (!function) {
        Logger::fatal(
            "reference exists, but is not a function: '" + 
                node.get_name() + "'", 
            node.get_span());
    }

    // Propogate the call expression reference.
    node.set_decl(decl);
    node.pType = function->get_type()->get_return_type();

    // Check that the number of arguments match the number of function params.
    if (node.num_args() != function->num_params()) {
        Logger::fatal(
            "call argument count mismatch, expected " + 
                std::to_string(function->num_params()), 
                node.get_span());
    }
}

void SymbolAnalysis::visit(RuneExpr& node) {
    for (auto& arg : node.rune()->args())
        arg->accept(*this);
}

void SymbolAnalysis::visit(RuneStmt& node) {
    for (auto& arg : node.rune()->args())
        arg->accept(*this);
}
