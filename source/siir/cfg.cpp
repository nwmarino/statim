#include "siir/cfg.hpp"
#include "siir/constant.hpp"
#include "siir/type.hpp"

using namespace stm;
using namespace stm::siir;

CFG::CFG(InputFile& file, Target& target) : m_file(file), m_target(target) {
    m_types_ints[IntegerType::TY_Int1] = new IntegerType(IntegerType::TY_Int1);
    m_types_ints[IntegerType::TY_Int8] = new IntegerType(IntegerType::TY_Int8);
    m_types_ints[IntegerType::TY_Int16] = new IntegerType(IntegerType::TY_Int16);
    m_types_ints[IntegerType::TY_Int32] = new IntegerType(IntegerType::TY_Int32);
    m_types_ints[IntegerType::TY_Int64] = new IntegerType(IntegerType::TY_Int64);
    m_types_floats[FloatType::TY_Float32] = new FloatType(FloatType::TY_Float32);
    m_types_floats[FloatType::TY_Float64] = new FloatType(FloatType::TY_Float64);

    m_int1_zero = new ConstantInt(0, m_types_ints[IntegerType::TY_Int1]);
    m_int1_one = new ConstantInt(1, m_types_ints[IntegerType::TY_Int1]);
}

CFG::~CFG() {
    for (auto [ name, global ] : m_globals) delete global;
    m_globals.clear();

    for (auto [ name, function ] : m_functions) delete function;
    m_functions.clear();

    for (auto [ kind, type ] : m_types_ints) delete type;
    m_types_ints.clear();

    for (auto [ kind, type ] : m_types_floats) delete type;
    m_types_floats.clear();

    for (auto [ element, size_pair ] : m_types_arrays) {
        for (auto [ size, type ] : size_pair)
            delete type;

        size_pair.clear();
    }

    m_types_arrays.clear();

    for (auto [ pointee, type ] : m_types_ptrs) delete type;
    m_types_ptrs.clear();

    for (auto [ name, type ] : m_types_structs) delete type;
    m_types_structs.clear();

    for (auto type : m_types_fns) delete type;
    m_types_fns.clear();

    if (m_int1_zero) {
        delete m_int1_zero;
        m_int1_zero = nullptr;
    }

    if (m_int1_one) {
        delete m_int1_one;
        m_int1_one = nullptr;
    }

    for (auto [ value, constant ] : m_pool_int8) delete constant;
    m_pool_int8.clear();

    for (auto [ value, constant ] : m_pool_int16) delete constant;
    m_pool_int16.clear();

    for (auto [ value, constant ] : m_pool_int32) delete constant;
    m_pool_int32.clear();

    for (auto [ value, constant ] : m_pool_int64) delete constant;
    m_pool_int64.clear();

    for (auto [ value, constant ] : m_pool_fp32) delete constant;
    m_pool_fp32.clear();

    for (auto [ value, constant ] : m_pool_fp64) delete constant;
    m_pool_fp64.clear();

    for (auto [ type, null ] : m_pool_null) delete null;
    m_pool_null.clear();

    for (auto [ block, addr ] : m_pool_baddr) delete addr;
    m_pool_baddr.clear();

    for (auto incoming : m_pool_incomings) delete incoming;
    m_pool_incomings.clear();
}

std::vector<StructType*> CFG::structs() const {
    std::vector<StructType*> structs;
    structs.reserve(m_types_structs.size());
    for (auto& [name, type] : m_types_structs)
        structs.push_back(type);

    return structs;
}

std::vector<Global*> CFG::globals() const {
    std::vector<Global*> globals;
    globals.reserve(m_globals.size());
    for (auto& [name, global] : m_globals)
        globals.push_back(global);

    return globals;
}

const Global* CFG::get_global(const std::string& name) const {
    auto it = m_globals.find(name);
    if (it != m_globals.end())
        return it->second;

    return nullptr;
}

void CFG::add_global(Global* glb) {
    assert(glb);
    assert(!get_global(glb->get_name()) && !get_function(glb->get_name())
        && "global has name conflicts with existing graph symbol");

    m_globals.emplace(glb->get_name(), glb);
    glb->set_parent(this);
}

void CFG::remove_global(Global* glb) {
    auto it = m_globals.find(glb->get_name());
    if (it != m_globals.end()) {
        assert(it->second == glb);
        assert(glb->get_parent() == this);

        m_globals.erase(it);
    }
}

std::vector<Function*> CFG::functions() const {
    std::vector<Function*> functions;
    functions.reserve(m_functions.size());
    for (auto& [name, function] : m_functions)
        functions.push_back(function);

    return functions;
}

const Function* CFG::get_function(const std::string& name) const {
    auto it = m_functions.find(name);
    if (it != m_functions.end())
        return it->second;

    return nullptr;
}

void CFG::add_function(Function* fn) {
    assert(fn);
    assert(!get_global(fn->get_name()) && !get_function(fn->get_name())
        && "function has name conflicts with existing graph symbol");

    m_functions.emplace(fn->get_name(), fn);
    fn->set_parent(this);
}

void CFG::remove_function(Function* fn) {
    auto it = m_functions.find(fn->get_name());
    if (it != m_functions.end()) {
        assert(it->second == fn);
        assert(fn->get_parent() == this);

        m_functions.erase(it);
    }
}
