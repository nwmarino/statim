#ifndef STATIM_SIIR_CFG_HPP_
#define STATIM_SIIR_CFG_HPP_

#include "siir/constant.hpp"
#include "siir/function.hpp"
#include "siir/global.hpp"
#include "siir/type.hpp"
#include "types/input_file.hpp"
#include "types/types.hpp"

#include <map>
#include <ostream>
#include <string>
#include <unordered_map>

namespace stm {

namespace siir {

/// The top-level SIIR control flow graph representation.
class CFG final {
    friend class ArrayType;
    friend class FunctionType;
    friend class PointerType;
    friend class StructType;
    friend class ConstantInt;
    friend class ConstantFP;
    friend class ConstantNull;

    /// Top-level graph items.
    InputFile& m_file;
    std::map<std::string, Global*> m_globals;
    std::map<std::string, Function*> m_functions;

    /// Type pooling.
    std::unordered_map<IntegerType::Kind, IntegerType*> m_types_ints;
    std::unordered_map<FloatType::Kind, FloatType*> m_types_floats;
    std::unordered_map<std::pair<const Type*, u32>, ArrayType*> m_types_arrays;
    std::unordered_map<const Type*, PointerType*> m_types_ptrs;
    std::map<std::string, StructType*> m_types_structs;
    std::vector<FunctionType*> m_types_fns; 

    /// Constant pooling.
    ConstantInt* m_int1_zero, m_int1_one;
    std::unordered_map<i8, ConstantInt*> m_pool_int8;
    std::unordered_map<i16, ConstantInt*> m_pool_int16;
    std::unordered_map<i32, ConstantInt*> m_pool_int32;
    std::unordered_map<i64, ConstantInt*> m_pool_int64;
    std::unordered_map<f32, ConstantFP*> m_pool_fp32;
    std::unordered_map<f64, ConstantFP*> m_pool_fp64;
    std::unordered_map<const Type*, ConstantNull*> m_pool_null;

public:
    CFG(InputFile& file);
    ~CFG();

    /// Get the input file that this control flow graph represents.
    const InputFile& get_file() const { return m_file; }
    InputFile& get_file() { return m_file; }
    void set_file(InputFile& file) { m_file = file; }

    /// Get the global in this graph with name \p name if it exists, and
    /// `nullptr` otherwise. 
    const Global* get_global(const std::string& name) const;
    Global* get_global(const std::string& name) {
        return const_cast<Global*>(
            static_cast<const CFG*>(this)->get_global(name));
    }

    /// Add a new global \p glb to this graph. Will fail if there is an 
    /// existing top-level value with the same name.
    void add_global(Global* glb);

    /// Get the function in this graph with name \p name if it exists, and
    /// `nullptr` otherwise.
    const Function* get_function(const std::string& name) const;
    Function* get_function(const std::string& name) {
        return const_cast<Function*>(
            static_cast<const CFG*>(this)->get_function(name));
    }

    /// Add a new function \p fn to this graph. Will fail if there is an
    /// existing top-level value with the same name.
    void add_function(Function* fn);

    const Type* get_int1_type() const;
    const Type* get_int8_type() const;
    const Type* get_int16_type() const;
    const Type* get_int32_type() const;
    const Type* get_int64_type() const;
    const Type* get_float32_type() const;
    const Type* get_float64_type() const;

    /// Print this graph in a reproducible plaintext format.
    void print(std::ostream& os);
};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_CFG_HPP_
