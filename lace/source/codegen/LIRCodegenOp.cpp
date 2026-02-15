//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Type.hpp"

#include "lir/graph/Constant.hpp"
#include "lir/graph/Instruction.hpp"
#include "lir/graph/Type.hpp"

using namespace lace;

lir::Value* LIRCodegen::codegen_assignment(const BinaryOp* expr) {
    lir::Value *ptr = codegen_addressed_expression(expr->get_lhs());
    assert(ptr);

    lir::Type* type = to_lir_type(expr->get_type());
    if (m_mach.is_scalar(type)) {
        lir::Value* value = codegen_valued_expression(expr->get_rhs());
        assert(value);
    
        m_builder.build_store(value, ptr);
        return value; // Return rhs as result of the assignment.
    } else {
        m_state.place = ptr;

        lir::Value* value = codegen_addressed_expression(expr->get_rhs());
        assert(value);

        if (value != ptr) {
            m_builder.build_call(getIntrinsicCopy(), {
                ptr,
                value,
                lir::Integer::get(m_cfg, lir::IntegerType::get(m_cfg, 64), m_mach.get_type_size(type) / 8),
            });
        }

        m_state.place = nullptr;
        return value;
    }
}

lir::Value *LIRCodegen::codegen_addition(const BinaryOp *expr) {
    assert(expr->get_operator() == BinaryOp::Add || 
        expr->get_operator() == BinaryOp::Sub);

    lir::Value *lhs = codegen_valued_expression(expr->get_lhs());
    lir::Value *rhs = codegen_valued_expression(expr->get_rhs());
    assert(lhs);
    assert(rhs);

    lir::Type *lhs_type = lhs->get_type();
    lir::Type *rhs_type = rhs->get_type();

    if (lhs_type->is_pointer_type() && rhs_type->is_integer_type()) {
        // Handle pointer arithmetic.
        if (expr->get_operator() == BinaryOp::Sub) {
            // For '-' pointer arithmetic, the index needs to be negated.
            if (auto integer = dynamic_cast<lir::Integer*>(rhs)) {
                rhs = lir::Integer::get(
                    m_cfg, 
                    rhs->get_type(), 
                    -integer->get_value()
                );
            } else {
                rhs = m_builder.build_ineg(rhs);
            }
        }

        return m_builder.build_offptr(lhs_type, lhs, rhs);
    } else if (lhs_type->is_integer_type() && rhs_type->is_integer_type()) {
        auto lhs_integer = dynamic_cast<lir::Integer*>(lhs);
        auto rhs_integer = dynamic_cast<lir::Integer*>(rhs);

        if (expr->get_operator() == BinaryOp::Add) {
            if (lhs_integer && rhs_integer) {
                return lir::Integer::get(
                    m_cfg, 
                    lhs_type, 
                    lhs_integer->get_value() + rhs_integer->get_value()
                );
            }

            return m_builder.build_iadd(lhs, rhs);
        } else {
            if (lhs_integer && rhs_integer) {
                return lir::Integer::get(
                    m_cfg, 
                    lhs_type, 
                    lhs_integer->get_value() - rhs_integer->get_value()
                );
            }

            return m_builder.build_isub(lhs, rhs);
        }
    } else if (lhs_type->is_float_type() && rhs_type->is_float_type()) {
        auto lhs_fp = dynamic_cast<lir::Float*>(lhs);
        auto rhs_fp = dynamic_cast<lir::Float*>(rhs);

        if (expr->get_operator() == BinaryOp::Add) {
            if (lhs_fp && rhs_fp) {
                return lir::Float::get(
                    m_cfg,
                    lhs_type,
                    lhs_fp->get_value() + rhs_fp->get_value()
                );
            }

            return m_builder.build_fadd(lhs, rhs);
        } else {
            if (lhs_fp && rhs_fp) {
                return lir::Float::get(
                    m_cfg,
                    lhs_type,
                    lhs_fp->get_value() - rhs_fp->get_value()
                );
            }

            return m_builder.build_fsub(lhs, rhs);
        }
    }

    assert(false && "invalid add/sub operation!");
}

lir::Value *LIRCodegen::codegen_multiply(const BinaryOp *expr) {
    assert(expr->get_operator() == BinaryOp::Mul);

    lir::Value *lhs = codegen_valued_expression(expr->get_lhs());
    lir::Value *rhs = codegen_valued_expression(expr->get_rhs());
    assert(lhs);
    assert(rhs);

    lir::Type *lhs_type = lhs->get_type();
    lir::Type *rhs_type = rhs->get_type();

    if (lhs_type->is_integer_type() && rhs_type->is_integer_type()) {
        auto lhs_integer = dynamic_cast<lir::Integer*>(lhs);
        auto rhs_integer = dynamic_cast<lir::Integer*>(rhs);

        if (lhs_integer && rhs_integer) {
            return lir::Integer::get(
                m_cfg, 
                lhs_type, 
                lhs_integer->get_value() * rhs_integer->get_value()
            );
        }

        return m_builder.build_imul(lhs, rhs);
    } else if (lhs_type->is_float_type() && rhs_type->is_float_type()) {
        auto lhs_fp = dynamic_cast<lir::Float*>(lhs);
        auto rhs_fp = dynamic_cast<lir::Float*>(rhs);

        if (lhs_fp && rhs_fp) {
            return lir::Float::get(
                m_cfg,
                lhs_type,
                lhs_fp->get_value() * rhs_fp->get_value()
            );
        }

        return m_builder.build_fmul(lhs, rhs);
    }

    assert(false && "invalid mul operation!");
}

lir::Value *LIRCodegen::codegen_division(const BinaryOp *expr) {
    assert(expr->get_operator() == BinaryOp::Div ||
        expr->get_operator() == BinaryOp::Mod);

    lir::Value *lhs = codegen_valued_expression(expr->get_lhs());
    lir::Value *rhs = codegen_valued_expression(expr->get_rhs());
    assert(lhs);
    assert(rhs);

    lir::Type *lhs_type = lhs->get_type();
    lir::Type *rhs_type = rhs->get_type();

    if (lhs_type->is_integer_type() && rhs_type->is_integer_type()) {
        auto lhs_integer = dynamic_cast<lir::Integer*>(lhs);
        auto rhs_integer = dynamic_cast<lir::Integer*>(rhs);

        if (lhs_integer && rhs_integer) {
            if (expr->get_operator() == BinaryOp::Div) {
                return lir::Integer::get(
                    m_cfg, 
                    lhs_type, 
                    lhs_integer->get_value() / rhs_integer->get_value()
                );
            } else if (expr->get_operator() == BinaryOp::Mod) {
                return lir::Integer::get(
                    m_cfg,
                    lhs_type,
                    lhs_integer->get_value() % rhs_integer->get_value()
                );
            }
        }
 
        if (expr->get_lhs()->get_type()->is_signed_integer()) {
            if (expr->get_operator() == BinaryOp::Div) {
                return m_builder.build_sdiv(lhs, rhs);
            } else if (expr->get_operator() == BinaryOp::Mod) {
                return m_builder.build_smod(lhs, rhs);
            }
        } else {
            if (expr->get_operator() == BinaryOp::Div) {
                return m_builder.build_udiv(lhs, rhs);
            } else if (expr->get_operator() == BinaryOp::Mod) {
                return m_builder.build_umod(lhs, rhs);
            }
        }

    } else if (lhs_type->is_float_type() && rhs_type->is_float_type()) {
        assert(expr->get_operator() == BinaryOp::Div && "fmod unsupported!");

        auto lhs_fp = dynamic_cast<lir::Float*>(lhs);
        auto rhs_fp = dynamic_cast<lir::Float*>(rhs);

        if (lhs_fp && rhs_fp) {
            return lir::Float::get(
                m_cfg,
                lhs_type,
                lhs_fp->get_value() / rhs_fp->get_value()
            );
        }

        return m_builder.build_fdiv(lhs, rhs);
    }

    assert(false && "invalid div/mod operation!");
}

lir::Value *LIRCodegen::codegen_bitwise_arithmetic(const BinaryOp *expr) {
    assert(expr->get_operator() == BinaryOp::And ||
        expr->get_operator() == BinaryOp::Or || 
        expr->get_operator() == BinaryOp::Xor);

    lir::Value *lhs = codegen_valued_expression(expr->get_lhs());
    lir::Value *rhs = codegen_valued_expression(expr->get_rhs());
    assert(lhs);
    assert(rhs);

    auto lhs_integer = dynamic_cast<lir::Integer*>(lhs);
    auto rhs_integer = dynamic_cast<lir::Integer*>(rhs);

    if (expr->get_operator() == BinaryOp::And) {
        if (lhs_integer && rhs_integer) {
            return lir::Integer::get(
                m_cfg, 
                lhs->get_type(), 
                lhs_integer->get_value() & rhs_integer->get_value()
            );
        }
        
        return m_builder.build_and(lhs, rhs);
    } else if (expr->get_operator() == BinaryOp::And) {
        if (lhs_integer && rhs_integer) {
            return lir::Integer::get(
                m_cfg, 
                lhs->get_type(), 
                lhs_integer->get_value() | rhs_integer->get_value()
            );
        }
        
        return m_builder.build_or(lhs, rhs);
    } else if (expr->get_operator() == BinaryOp::And) {
        if (lhs_integer && rhs_integer) {
            return lir::Integer::get(
                m_cfg, 
                lhs->get_type(), 
                lhs_integer->get_value() ^ rhs_integer->get_value()
            );
        }
        
        return m_builder.build_xor(lhs, rhs);
    }

    assert(false && "invalid and/or/xor operation!");
}

lir::Value *LIRCodegen::codegen_bit_shift(const BinaryOp *expr) {
    assert(expr->get_operator() == BinaryOp::LShift ||
        expr->get_operator() == BinaryOp::RShift);

    lir::Value *lhs = codegen_valued_expression(expr->get_lhs());
    lir::Value *rhs = codegen_valued_expression(expr->get_rhs());
    assert(lhs);
    assert(rhs);
    assert(lhs->get_type()->is_integer_type() && 
        rhs->get_type()->is_integer_type());

    auto lhs_integer = dynamic_cast<lir::Integer*>(lhs);
    auto rhs_integer = dynamic_cast<lir::Integer*>(rhs);

    if (expr->get_operator() == BinaryOp::LShift) {
        if (lhs_integer && rhs_integer) {
            return lir::Integer::get(
                m_cfg, 
                lhs->get_type(), 
                lhs_integer->get_value() << rhs_integer->get_value()
            );
        }
        
        return m_builder.build_shl(lhs, rhs);
    } else if (expr->get_operator() == BinaryOp::RShift) {
        if (lhs_integer && rhs_integer) {
            return lir::Integer::get(
                m_cfg, 
                lhs->get_type(), 
                lhs_integer->get_value() | rhs_integer->get_value()
            );
        }
        
        if (expr->get_lhs()->get_type()->is_signed_integer()) {
            return m_builder.build_sar(lhs, rhs); 
        } else {
            return m_builder.build_shr(lhs, rhs);
        }
    }

    assert(false && "invalid ls/rs operation!");
}

lir::Value *LIRCodegen::codegen_numerical_comparison(const BinaryOp *expr) {
    lir::Value *lhs = codegen_valued_expression(expr->get_lhs());
    lir::Value *rhs = codegen_valued_expression(expr->get_rhs());
    assert(lhs);
    assert(rhs);

    // @Todo: Implement constant folding here.

    const QualType &type = expr->get_lhs()->get_type();
    switch (expr->get_operator()) {
        case BinaryOp::Eq:
            if (type->is_integer() || type->isClass(Type::Class::Pointer)) {
                return m_builder.build_cmp_ieq(lhs, rhs);
            } else if (type->is_floating_point()) {
                return m_builder.build_cmp_feq(lhs, rhs);
            }

        case BinaryOp::NEq:
            if (type->is_integer() || type->isClass(Type::Class::Pointer)) {
                return m_builder.build_cmp_ine(lhs, rhs);
            } else if (type->is_floating_point()) {
                return m_builder.build_cmp_fne(lhs, rhs);
            }

        case BinaryOp::Lt:
            if (type->is_signed_integer() || type->isClass(Type::Class::Pointer)) {
                return m_builder.build_cmp_slt(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
                return m_builder.build_cmp_ult(lhs, rhs);
            } else if (type->is_floating_point()) {
                return m_builder.build_cmp_flt(lhs, rhs);
            }

        case BinaryOp::LtEq:
            if (type->is_signed_integer() || type->isClass(Type::Class::Pointer)) {
                return m_builder.build_cmp_sle(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
                return m_builder.build_cmp_ule(lhs, rhs);
            } else if (type->is_floating_point()) {
                return m_builder.build_cmp_fle(lhs, rhs);
            }

        case BinaryOp::Gt:
            if (type->is_signed_integer() || type->isClass(Type::Class::Pointer)) {
                return  m_builder.build_cmp_sgt(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
                return  m_builder.build_cmp_ugt(lhs, rhs);
            } else if (type->is_floating_point()) {
                return  m_builder.build_cmp_fgt(lhs, rhs);
            }

        case BinaryOp::GtEq:
            if (type->is_signed_integer() || type->isClass(Type::Class::Pointer)) {
                return m_builder.build_cmp_sge(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
                return m_builder.build_cmp_uge(lhs, rhs);
            } else if (type->is_floating_point()) {
                return m_builder.build_cmp_fge(lhs, rhs);
            }

        default:
            break;
    }

    assert(false && "invalid cmp operator!");
}

lir::Value *LIRCodegen::codegen_logical_and(const BinaryOp *expr) {
    lir::BasicBlock *right_bb = lir::BasicBlock::create();
    lir::BasicBlock *merge_bb = lir::BasicBlock::create();

    lir::Value *lhs = codegen_valued_expression(expr->get_lhs());
    assert(lhs);
    lhs = inject_comparison(lhs);

    lir::BasicBlock *false_bb = m_builder.get_insert();
    m_builder.build_brif(inject_comparison(lhs), right_bb, merge_bb);

    m_func->append(right_bb);
    m_builder.set_insert(right_bb);

    lir::Value *rhs = codegen_valued_expression(expr->get_rhs());
    assert(rhs);
    rhs = inject_comparison(rhs);

    m_builder.build_jump(merge_bb);

    lir::BasicBlock *otherwise_bb = m_builder.get_insert();
    m_func->append(merge_bb);
    m_builder.set_insert(merge_bb);

    lir::Phi *phi = m_builder.build_phi(to_lir_type(expr->get_type()));
    phi->add_edge(lir::Integer::get_false(m_cfg), false_bb);
    phi->add_edge(rhs, otherwise_bb);

    return phi;
}

lir::Value *LIRCodegen::codegen_logical_or(const BinaryOp *expr) {
    lir::BasicBlock *right_bb = lir::BasicBlock::create();
    lir::BasicBlock *merge_bb = lir::BasicBlock::create();

    lir::Value *lhs = codegen_valued_expression(expr->get_lhs());
    assert(lhs);
    lhs = inject_comparison(lhs);

    lir::BasicBlock *true_bb = m_builder.get_insert();
    m_builder.build_brif(lhs, merge_bb, right_bb);

    m_func->append(right_bb);
    m_builder.set_insert(right_bb);

    lir::Value *rhs = codegen_valued_expression(expr->get_rhs());
    assert(rhs);
    rhs = inject_comparison(rhs);

    m_builder.build_jump(merge_bb);

    lir::BasicBlock *otherwise_bb = m_builder.get_insert();
    m_func->append(merge_bb);
    m_builder.set_insert(merge_bb);

    lir::Phi *phi = m_builder.build_phi(to_lir_type(expr->get_type()));
    phi->add_edge(lir::Integer::get_true(m_cfg), true_bb);
    phi->add_edge(rhs, otherwise_bb);
    
    return phi;
}

lir::Value *LIRCodegen::codegen_negation(const UnaryOp *expr) {
    lir::Value* value = codegen_valued_expression(expr->get_expr());
    assert(value);

    lir::Type *type = value->get_type();
    if (type->is_integer_type()) {
        if (auto integer = dynamic_cast<lir::Integer*>(value)) {
            return lir::Integer::get(m_cfg, type, -integer->get_value());
        } else {
            return m_builder.build_ineg(value);
        }
    } else if (type->is_float_type()) {
        if (auto fp = dynamic_cast<lir::Float*>(value)) {
            return lir::Float::get(m_cfg, type, -fp->get_value());
        } else {
            return m_builder.build_fneg(value);
        }
    }
     
    assert(false && "invalid negate operation!");
}

lir::Value *LIRCodegen::codegen_bitwise_not(const UnaryOp *expr) {
    lir::Value *value = codegen_valued_expression(expr->get_expr());
    assert(value);

    if (value->get_type()->is_integer_type()) {
        if (auto integer = dynamic_cast<lir::Integer*>(value)) {
            return lir::Integer::get(m_cfg, value->get_type(), ~integer->get_value());
        } else {
            return m_builder.build_not(value);
        }
    }

    assert(false && "invalid bitwise not operation!");
}

lir::Value *LIRCodegen::codegen_logical_not(const UnaryOp *expr) {
    lir::Value *value = codegen_valued_expression(expr->get_expr());
    assert(value);

    lir::Type *type = value->get_type();
    if (type->is_integer_type()) {
        if (auto integer = dynamic_cast<lir::Integer*>(value)) {
            return lir::Integer::get(
                m_cfg, 
                lir::Type::get_i8(m_cfg), 
                !integer->get_value()
            );
        }

        return m_builder.build_cmp_ieq(value, lir::Integer::get_zero(m_cfg, type));
    } else if (type->is_float_type()) {
        if (auto fp = dynamic_cast<lir::Float*>(value)) {
            return lir::Integer::get(
                m_cfg, 
                lir::Type::get_i8(m_cfg), 
                !fp->get_value()
            );
        }

        return m_builder.build_cmp_feq(value, lir::Float::get_zero(m_cfg, type));
    } else if (type->is_pointer_type()) {
        if (dynamic_cast<lir::Null*>(value)) {
            return lir::Integer::get_true(m_cfg); 
        }

        return m_builder.build_cmp_ieq(value, lir::Null::get(m_cfg, type));
    }

    assert(false && "invalid logical not operation!");
}

lir::Value *LIRCodegen::codegen_address_of(const UnaryOp *expr) {
    return codegen_addressed_expression(expr->get_expr());
}
