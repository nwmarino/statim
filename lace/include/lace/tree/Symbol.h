//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_SYMBOL_H_
#define LACE_SYMBOL_H_

#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

#include <cstdint>
#include <string>

namespace lace {

struct Symbol final {
    enum class Kind : uint32_t {
        Definition,
        Type,
    };

    enum class Visibility : uint32_t {
        Public,
        Private,
    };

    std::string name;
    std::string qual_name;
    Kind kind;
    Visibility visibility;
    union {
        NamedDefn* defn;
        Type* type;
    };
};

} // namespace lace

#endif // LACE_SYMBOL_H_
