//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/AMD64.h"
#include "lir/machine/FunctionABI.h"

using namespace lir;

FunctionABI::FunctionABI(const Machine& mach, const Function* func) {
    const FunctionType* type = func->get_type();
    int32_t offset = 0; // @Todo: changes with architecture and system ABI.

    if (type->has_result()) {
        const Type* result = type->get_result();
        assert(mach.is_scalar(result));

        Register rReg;

        // @Todo: change with architecture.
        if (result->is_float_type()) {
            rReg = AMD64_Register::XMM0;
        } else {
            rReg = AMD64_Register::RAX;
        }

        m_result = Location { Location::Kind::Register, rReg };
    }

    for (const Type* param : type->get_params()) {
        m_params.push_back(Location {
            .kind = Location::Kind::Stack,
            .offset = offset
        });

        offset += (mach.get_type_size(param) / 8);
    }
}
