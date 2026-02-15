//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/LIRCodegen.h"
#include "lace/tree/Type.h"

#include "lir/graph/Type.hpp"

using namespace lace;

lir::Value *LIRCodegen::codegen_valued_expression(const Expr *expr) {
    switch (expr->get_kind()) {
        case Expr::Bool:
            return codegen_literal_boolean(static_cast<const BoolLiteral*>(expr));
        case Expr::Char:
            return codegen_literal_character(static_cast<const CharLiteral*>(expr));
        case Expr::Integer:
            return codegen_literal_integer(static_cast<const IntegerLiteral*>(expr));
        case Expr::Float:
            return codegen_literal_float(static_cast<const FloatLiteral*>(expr));
        case Expr::Null:
            return codegen_literal_null(static_cast<const NullLiteral*>(expr));
        case Expr::String:
            return codegen_literal_string(static_cast<const StringLiteral*>(expr));

        case Expr::Binary: {
            auto binary = static_cast<const BinaryOp*>(expr);

            switch (binary->get_operator()) {
                case BinaryOp::Assign:
                    return codegen_assignment(binary);
                case BinaryOp::Add:
                case BinaryOp::Sub:
                    return codegen_addition(binary);
                case BinaryOp::Mul:
                    return codegen_multiply(binary);
                case BinaryOp::Div:
                case BinaryOp::Mod:
                    return codegen_division(binary);
                case BinaryOp::And:
                case BinaryOp::Or:
                case BinaryOp::Xor:
                    return codegen_bitwise_arithmetic(binary);
                case BinaryOp::LShift:
                case BinaryOp::RShift:
                    return codegen_bit_shift(binary);
                case BinaryOp::LogicAnd:
                    return codegen_logical_and(binary);
                case BinaryOp::LogicOr:
                    return codegen_logical_or(binary);
                case BinaryOp::Eq:
                case BinaryOp::NEq:
                case BinaryOp::Lt:
                case BinaryOp::LtEq:
                case BinaryOp::Gt:
                case BinaryOp::GtEq:
                    return codegen_numerical_comparison(binary);
                default:
                    assert(false && "unknown binary operator!");
            }
        }

        case Expr::Unary: {
            auto unary = static_cast<const UnaryOp*>(expr);

            switch (unary->get_operator()) {
                case UnaryOp::Negate:
                    return codegen_negation(unary);
                case UnaryOp::Not:
                    return codegen_bitwise_not(unary);
                case UnaryOp::LogicNot:
                    return codegen_logical_not(unary);
                case UnaryOp::AddressOf:
                    return codegen_address_of(unary);
                case UnaryOp::Dereference:
                    return codegen_valued_dereference(unary);
                default:
                    assert(false && "unknown unary operator!");
            }
        }
        
        case Expr::Access:
            return codegen_valued_access(static_cast<const AccessExpr*>(expr));
        case Expr::Ref:
            return codegen_valued_reference(static_cast<const RefExpr*>(expr));
        case Expr::Subscript:
            return codegen_valued_subscript(static_cast<const SubscriptExpr*>(expr));
        case Expr::Call:
            return codegen_function_call(static_cast<const CallExpr*>(expr));
        case Expr::Cast:
            return codegen_type_cast(static_cast<const CastExpr*>(expr));
        case Expr::Paren:
            return codegen_parentheses(static_cast<const ParenExpr*>(expr));
        case Expr::Sizeof:
            return codegen_sizeof(static_cast<const SizeofExpr*>(expr));
    }
}

lir::Value *LIRCodegen::codegen_valued_access(const AccessExpr *expr) {
    lir::Value *addr = codegen_addressed_access(expr);
    assert(addr);

    return m_builder.build_load(to_lir_type(expr->get_type()), addr);
}

lir::Value *LIRCodegen::codegen_valued_reference(const RefExpr *expr) {
    assert(expr->get_defn());

    switch (expr->get_defn()->get_kind()) {
        case Defn::Parameter:
        case Defn::Variable: {
            lir::Value *addr = codegen_addressed_reference(expr);
            assert(addr);

            return m_builder.build_load(to_lir_type(expr->get_type()), addr);
        }
    
        case Defn::Variant: {
            auto var = static_cast<const VariantDefn*>(expr->get_defn());
            return lir::Integer::get(
                m_cfg, 
                to_lir_type(expr->get_type()), 
                var->get_value()
            );
        }

        default:
            assert(false && "unable to generate valued reference!");
    }
}

lir::Value *LIRCodegen::codegen_valued_subscript(const SubscriptExpr *expr) {
    lir::Value *addr = codegen_addressed_subscript(expr);
    assert(addr);

    return m_builder.build_load(to_lir_type(expr->get_type()), addr);
}

lir::Value* LIRCodegen::codegen_valued_dereference(const UnaryOp* expr) {
    lir::Value* ptr = codegen_addressed_dereference(expr);
    assert(ptr);

    return m_builder.build_load(to_lir_type(expr->get_type()), ptr);
}

lir::Value* LIRCodegen::codegen_literal_boolean(const BoolLiteral* expr) {
    return lir::Integer::get(
        m_cfg, 
        lir::Type::get_i8(m_cfg),
        static_cast<int64_t>(expr->get_value())
    );
}

lir::Value* LIRCodegen::codegen_literal_integer(const IntegerLiteral* expr) {
    return lir::Integer::get(
        m_cfg,
        to_lir_type(expr->get_type()),
        expr->get_value()
    );
}

lir::Value* LIRCodegen::codegen_literal_character(const CharLiteral* expr) {
    return lir::Integer::get(
        m_cfg,
        lir::Type::get_i8(m_cfg),
        static_cast<int64_t>(expr->get_value())
    );
}

lir::Value* LIRCodegen::codegen_literal_float(const FloatLiteral* expr) {
    return lir::Float::get(
        m_cfg,
        to_lir_type(expr->get_type()),
        static_cast<double>(expr->get_value())
    );
}

lir::Value* LIRCodegen::codegen_literal_null(const NullLiteral* expr) {
    return lir::Null::get(m_cfg, to_lir_type(expr->get_type()));
}

lir::Value* LIRCodegen::codegen_literal_string(const StringLiteral* expr) {
    return m_builder.build_string(lir::String::get(m_cfg, expr->get_value()));
}

lir::Value* LIRCodegen::codegen_type_cast(const CastExpr* expr) {
    lir::Value* value = codegen_valued_expression(expr->get_expr());
    assert(value);

    lir::Type* source = value->get_type();
    lir::Type* dest = to_lir_type(expr->get_type());

    if (source->is_integer_type()) {
        if (dest->is_integer_type()) {
            // Handle integer -> integer type casts.
            if (auto integer = dynamic_cast<lir::Integer*>(value)) {
                // @Todo: check if the constant integer value actually fits 
                // within the destination type. Or, let the backend handle it
                // as undefined behaviour.
                return lir::Integer::get(m_cfg, dest, integer->get_value());
            }

            const uint32_t source_size = m_mach.get_type_size(source);
            const uint32_t dest_size = m_mach.get_type_size(dest);

            if (source_size == dest_size) {
                return value;
            } else if (source_size > dest_size) {
                return m_builder.build_itrunc(dest, value);
            }
            
            if (expr->get_expr()->get_type()->is_signed_integer()) {
                return m_builder.build_sext(dest, value);
            } else {
                return m_builder.build_zext(dest, value);
            }
        } else if (dest->is_float_type()) {
            // Handle integer -> floating point type casts.
            if (auto integer = dynamic_cast<lir::Integer*>(value))
                return lir::Float::get(m_cfg, dest, integer->get_value());

            if (expr->get_expr()->get_type()->is_signed_integer()) {
                return m_builder.build_s2f(dest, value);
            } else {
                return m_builder.build_u2f(dest, value);
            }
        } else if (dest->is_pointer_type()) {
            // Handle integer -> pointer type casts.
            if (auto integer = dynamic_cast<lir::Integer*>(value)) {
                // Fold cast<*T>(0) to null.
                if (integer->get_value() == 0)
                    return lir::Null::get(m_cfg, dest);
            }

            return m_builder.build_i2p(dest, value);
        }
    } else if (source->is_float_type()) {
        if (dest->is_integer_type()) {
            // Handle floating point -> integer type casts.
            if (auto fp = dynamic_cast<lir::Float*>(value)) {
                return lir::Integer::get(m_cfg, dest, fp->get_value());
            }

            if (expr->get_type()->is_signed_integer()) {
                return m_builder.build_f2s(dest, value);
            } else {
                return m_builder.build_f2u(dest, value);
            }
        } else if (dest->is_float_type()) {
            // Handle floating point -> floating point type casts.
            if (auto fp = dynamic_cast<lir::Float*>(value)) {
                return lir::Float::get(m_cfg, dest, fp->get_value());
            }

            const uint32_t source_size = m_mach.get_type_size(source);
            const uint32_t dest_size = m_mach.get_type_size(dest);

            if (source_size == dest_size) {
                return value;
            } else if (source_size > dest_size) {
                return m_builder.build_ftrunc(dest, value);
            } else {
                return m_builder.build_fext(dest, value);
            }
        }
    } else if (source->is_array_type()) {
        if (dest->is_pointer_type()) {
            return m_builder.build_reint(dest, value);
        }
    } else if (source->is_pointer_type()) {
        if (dest->is_integer_type()) {
            // Handle pointer -> integer type casts.
            if (auto null = dynamic_cast<lir::Null*>(value)) {
                // Fold null to 0s.
                return lir::Integer::get_zero(m_cfg, dest);
            }

            return m_builder.build_p2i(dest, value);
        } else if (dest->is_pointer_type()) {
            // Handle pointer -> pointer type casts.
            if (dynamic_cast<lir::Null*>(value)) {
                return lir::Null::get(m_cfg, dest);
            }

            return m_builder.build_reint(dest, value);
        }
    }
    
    log::fatal("unsupported type cast", 
        log::Span(m_ast->get_file(), expr->get_span()));
}

lir::Value* LIRCodegen::codegen_function_call(const CallExpr* expr) {
    lir::Value* callee = codegen_addressed_expression(expr->get_callee());
    assert(callee);

    std::vector<lir::Value*> args = {};
    args.reserve(expr->num_args());

    lir::Value* aret = nullptr;
    lir::Type* result_type = to_lir_type(expr->get_type());
    if (!m_mach.is_scalar(result_type)) {
        if (m_state.place) {
            aret = m_state.place;
        } else {
            aret = lir::Local::create(
                m_cfg, 
                result_type, 
                std::to_string(m_cfg.get_def_id()),
                m_func
            );
        }
        
        args.push_back(aret);
    }

    for (Expr* arg : expr->get_args()) {
        lir::Value* value = nullptr;
        lir::Type* type = to_lir_type(arg->get_type());

        if (!m_mach.is_scalar(type)) {
            value = codegen_addressed_expression(arg);
        } else {
            value = codegen_valued_expression(arg);
        }
        
        assert(value);

        args.push_back(value);
    }

    lir::Call* call = m_builder.build_call(dynamic_cast<lir::Function*>(callee), args);
    
    if (aret) {
        return aret;
    } else {
        return call;
    }
}

lir::Value* LIRCodegen::codegen_parentheses(const ParenExpr* expr) {
    return codegen_valued_expression(expr->get_expr());
}

lir::Value* LIRCodegen::codegen_sizeof(const SizeofExpr* expr) {
    return lir::Integer::get(
        m_cfg,
        to_lir_type(expr->get_type()),
        (m_mach.get_type_size(to_lir_type(expr->get_target_type())) / 8)
    );
}
