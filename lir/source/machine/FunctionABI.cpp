//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/FunctionABI.hpp"

using namespace lir;

/// Aligns the given |offset| to the provided |alignment|.
static inline uint32_t align_to(uint32_t offset, uint32_t alignment) {
    return (offset + alignment - 1) & ~(alignment - 1);
}

FunctionABI::FunctionABI(const Machine &mach, const Function *func) {
    const FunctionType *type = func->get_type();
    int32_t offset = 16;

    if (type->has_result()) {
        const Type *result = type->get_result();
        m_result = Location { .kind = Location::Kind::Stack, .offset = offset };
        offset += mach.get_type_size(result) / 8;
        //offset = align_to(offset + mach.get_type_size(result) / 8, 16);
    }

    for (const Type *param : type->get_params()) {
        m_params.push_back(Location {
            .kind = Location::Kind::Stack,
            .offset = offset
        });

        offset += (mach.get_type_size(param) / 8);
        //offset = align_to(offset, 16);
    }
}
