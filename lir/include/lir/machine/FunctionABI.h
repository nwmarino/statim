//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_FUNCTION_ABI_H_
#define LIR_FUNCTION_ABI_H_

#include "lir/graph/Function.h"
#include "lir/machine/Machine.h"
#include "lir/machine/Register.h"

#include <cstdint>
#include <optional>

namespace lir {

/// Descriptor for function ABIs regarding positions of arguments and results.
class FunctionABI final {
public:
    struct Location final {
        enum class Kind : uint32_t { Register, Stack };
        
        Kind kind;
        union {
            Register reg;
            int32_t offset;
        };
    };

private:
    std::vector<Location> m_params = {};
    std::optional<Location> m_result = std::nullopt;

public:
    FunctionABI(const Machine& mach, const Function* func);

    /// Test if this ABI contains any parameter locations.
    bool hasParams() const { return !m_params.size(); }

    /// Returns the number of parameters in the ABI for this function.
    uint32_t numParams() const { return m_params.size(); }

    /// Returns the ABI location for the parameter at the given |index|.
    const Location& getParamLocation(uint32_t index) const {
        assert(index < m_params.size() && "index out of bounds!");
        return m_params[index];
    }

    /// Test if this ABI provides a result value.
    bool hasResult() const { return m_result.has_value(); }

    const Location& getResultLocation() const {
        assert(hasResult() && "function does not have a result!");
        return *m_result;
    }
};

} // namespace lir

#endif // LIR_FUNCTION_ABI_H_
