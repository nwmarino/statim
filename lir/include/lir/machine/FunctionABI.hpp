//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_FUNCTION_ABI_H_
#define LIR_FUNCTION_ABI_H_

#include "lir/graph/Function.hpp"
#include "lir/machine/Machine.hpp"
#include "lir/machine/Register.hpp"

#include <cstdint>
#include <optional>

namespace lir {

/// Descriptor for function ABIs regarding positions of argument and results.
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
    FunctionABI(const Machine &mach, const Function *func);

    /// Test if there are any parameters to this function.
    bool has_params() const { return !m_params.size(); }

    /// Returns the number of parameters in the ABI for this function.
    uint32_t num_params() const { return m_params.size(); }

    const Location &get_param_location(uint32_t index) const {
        assert(index < m_params.size() && "index out of bounds!");
        return m_params[index];
    }

    /// Test if this ABI provides a result value.
    bool has_result() const { return m_result.has_value(); }

    const Location &get_result_location() const {
        assert(has_result() && "function does not have a result!");
        return *m_result;
    }
};

} // namespace lir

#endif // LIR_FUNCTION_ABI_H_
