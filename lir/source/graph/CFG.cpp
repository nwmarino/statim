//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/CFG.h"
#include "lir/graph/Constant.h"
#include "lir/graph/Type.h"
#include "lir/graph/Value.h"

#include <format>

using namespace lir;

CFG::CFG(const Machine &mach, const std::string &filename) 
  : m_mach(mach), m_filename(filename) {
    m_types.void_type = new VoidType();

    m_types.ints.emplace(8, new IntegerType(8));
    m_types.ints.emplace(16, new IntegerType(16));
    m_types.ints.emplace(32, new IntegerType(32));
    m_types.ints.emplace(64, new IntegerType(64));
    
    m_types.floats.emplace(32, new FloatType(32));
    m_types.floats.emplace(64, new FloatType(64));
}

CFG::~CFG() {
    for (auto &pair : m_globals)
        delete pair.second;

    for (auto &pair : m_functions)
        delete pair.second;

    m_globals.clear();
    m_functions.clear();

    if (m_types.void_type)
        delete m_types.void_type;

    for (auto &pair : m_types.ints)
        delete pair.second;

    for (auto &pair : m_types.floats)
        delete pair.second;

    for (auto &[element, size_pair] : m_types.arrays) {
        for (auto &[size, type] : size_pair)
            delete type;

        size_pair.clear();
    }

    for (auto &pair : m_types.pointers)
        delete pair.second;

    for (auto &pair : m_types.structs)
        delete pair.second;

    for (auto &type : m_types.functions)
        delete type;

    m_types.void_type = nullptr;
    m_types.arrays.clear();
    m_types.floats.clear();
    m_types.functions.clear();
    m_types.ints.clear();
    m_types.pointers.clear();
    m_types.structs.clear();

    for (auto &pair : m_constants.bytes)
        delete pair.second;

    for (auto &pair : m_constants.shorts)
        delete pair.second;

    for (auto &pair : m_constants.ints) 
        delete pair.second;

    for (auto &pair : m_constants.longs)
        delete pair.second;

    for (auto &pair : m_constants.floats)
        delete pair.second;

    for (auto &pair : m_constants.doubles)
        delete pair.second;

    for (auto &pair : m_constants.nulls)
        delete pair.second;

    for (auto &pair : m_constants.strings)
        delete pair.second;
}

std::vector<StructType*> CFG::get_structs() const {
    std::vector<StructType*> structs;
    structs.reserve(m_types.structs.size());
    for (const auto &pair : m_types.structs)
        structs.push_back(pair.second);

    return structs;
}

std::vector<Global*> CFG::get_globals() const {
    std::vector<Global*> globals;
    globals.reserve(m_globals.size());

    for (const auto &pair : m_globals)
        globals.push_back(pair.second);

    return globals;
}

const Global *CFG::get_global(const std::string &name) const {
    auto it = m_globals.find(name);
    if (it != m_globals.end())
        return it->second;

    return nullptr;
}

void CFG::add_global(Global *global) {
    assert(global && "global cannot be null!");
    assert(!get_global(global->get_name()) && 
        "a global with the same name already exists!");

    m_globals.emplace(global->get_name(), global);
    global->set_parent(this);
}

void CFG::remove_global(Global *global) {
    auto it = m_globals.find(global->get_name());
    if (it != m_globals.end()) {
        assert(it->second == global && 
            "multiple globals exist with the same name!");
        assert(global->get_parent() == this &&
            "global belongs to the wrong graph!");

        m_globals.erase(it);
    }
}

std::vector<Function*> CFG::get_functions() const {
    std::vector<Function*> functions;
    functions.reserve(m_functions.size());

    for (const auto &pair : m_functions)
        functions.push_back(pair.second);

    return functions;
}

const Function *CFG::get_function(const std::string &name) const {
    auto it = m_functions.find(name);
    if (it != m_functions.end())
        return it->second;

    return nullptr;
}

void CFG::add_function(Function *func) {
    assert(func && "function cannot be null!");
    assert(!get_function(func->get_name()) && 
        "a function with the same name already exists!");

    m_functions.emplace(func->get_name(), func);
    func->set_parent(this);
}

void CFG::remove_function(Function *func) {
    auto it = m_functions.find(func->get_name());
    if (it != m_functions.end()) {
        assert(it->second == func && 
            "multiple functions exist with the same name!");
        assert(func->get_parent() == this &&
            "function belongs to the wrong graph!");

        m_functions.erase(it);
    }
}

void CFG::print(std::ostream &os) const {
    for (const auto &pair : m_types.structs) {
        const StructType *type = pair.second;

        os << std::format("{} :: {{ ", type->get_name());

        for (uint32_t i = 0, e = type->num_fields(); i < e; ++i) {
            os << type->get_field(i)->to_string();
            if (i + 1 != e)
                os << ", ";
        }

        os << " }\n";
    }

    if (!m_types.structs.empty())
        os << '\n';

    for (const auto &pair : m_globals) {
        pair.second->print(os, PrintPolicy::Def);
        os << '\n';
    }

    if (!m_globals.empty())
        os << '\n';

    uint32_t i = 0, e = m_functions.size();
    for (const auto &pair : m_functions) {
        pair.second->print(os, PrintPolicy::Def);

        // Don't print double empty lines at the end of a graph print.
        if (++i != e)
            os << '\n';
    }

    if (!m_functions.empty())
        os << '\n';

    for (const DebugNode* node : m_debug) {
        node->print(os, PrintPolicy::Def);
    }
}
