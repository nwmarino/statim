#ifndef STATIM_SIIR_CFG_HPP_
#define STATIM_SIIR_CFG_HPP_

#include "siir/basicblock.hpp"
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

class Target;

/// The top-level SIIR control flow graph.
class CFG final {
    friend class Type;
    friend class ArrayType;
    friend class FunctionType;
    friend class PointerType;
    friend class StructType;
    friend class ConstantInt;
    friend class ConstantFP;
    friend class ConstantNull;
    friend class BlockAddress;
    friend class ConstantString;
    friend class Instruction;

    /// Top-level graph items.
    InputFile& m_file;
    Target& m_target;
    u32 m_def_id = 1;
    std::map<std::string, Global*> m_globals = {};
    std::map<std::string, Function*> m_functions = {};

    /// Type pooling.
    std::unordered_map<IntegerType::Kind, IntegerType*> m_types_ints = {};
    std::unordered_map<FloatType::Kind, FloatType*> m_types_floats = {};
    std::unordered_map<const Type*, 
        std::unordered_map<u32, ArrayType*>> m_types_arrays = {};
    std::unordered_map<const Type*, PointerType*> m_types_ptrs = {};
    std::map<std::string, StructType*> m_types_structs = {};
    std::vector<FunctionType*> m_types_fns = {}; 

    /// Constant pooling.
    ConstantInt *m_int1_zero, *m_int1_one;
    std::unordered_map<i8, ConstantInt*> m_pool_int8 = {};
    std::unordered_map<i16, ConstantInt*> m_pool_int16 = {};
    std::unordered_map<i32, ConstantInt*> m_pool_int32 = {};
    std::unordered_map<i64, ConstantInt*> m_pool_int64 = {};
    std::unordered_map<f32, ConstantFP*> m_pool_fp32 = {};
    std::unordered_map<f64, ConstantFP*> m_pool_fp64 = {};
    std::unordered_map<const Type*, ConstantNull*> m_pool_null = {};
    std::unordered_map<const BasicBlock*, BlockAddress*> m_pool_baddr = {};
    std::unordered_map<std::string, ConstantString*> m_pool_str = {};

    /// PHI operand pooling. This is only here because the memory cannot be 
    /// appropriately managed by the individual instructions 
    std::vector<PhiOperand*> m_pool_incomings = {};

public:
    /// Create a new control flow graph, representing |file| with the given
    /// target. Note that the target cannot be mutated later.
    CFG(InputFile& file, Target& target);
    
    CFG(const CFG&) = delete;
    CFG& operator = (const CFG&) = delete;
    
    ~CFG();

    /// Returns the input file that this control flow graph represents.
    const InputFile& get_file() const { return m_file; }
    InputFile& get_file() { return m_file; }

    /// Set the file that this graph represents to |file|.
    void set_file(InputFile& file) { m_file = file; }

    /// Returns the target of this control flow graph.
    const Target& get_target() const { return m_target; }
    Target& get_target() { return m_target; }

    /// Return a list of all the structure types in this graph, in order of
    /// creation.
    std::vector<StructType*> structs() const;

    /// Returns a list of all functions in this graph, in order of addition.
    std::vector<Global*> globals() const;

    /// Returns the global in this graph with the provided name, if it exists, 
    /// and null otherwise. 
    const Global* get_global(const std::string& name) const;
    Global* get_global(const std::string& name) {
        return const_cast<Global*>(
            static_cast<const CFG*>(this)->get_global(name));
    }

    /// Add |glb| to this graph. Fails if there is any existing top-level value 
    /// with the same name.
    void add_global(Global* glb);

    /// Remove |glb| if it exists in this graph.
    void remove_global(Global* glb);

    /// Returns a list of all functions in this graph, in order of addition.
    std::vector<Function*> functions() const;

    /// Returns the function in this graph with the provided name if it exists, 
    /// and null otherwise.
    const Function* get_function(const std::string& name) const;
    Function* get_function(const std::string& name) {
        return const_cast<Function*>(
            static_cast<const CFG*>(this)->get_function(name));
    }

    /// Add |fn| to this graph. Fails if there is any existing top-level value 
    /// with the same name.
    void add_function(Function* fn);

    /// Remove |fn| if it exists in this graph.
    void remove_function(Function* fn);

    /// Return a new unique definition id to create an instruction with.
    u32 get_def_id() { return m_def_id++; }

    /// Print this graph in a reproducible plaintext format to the output
    /// stream |os|.
    void print(std::ostream& os) const;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_CFG_HPP_
