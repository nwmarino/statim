//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/CFG.h"
#include "lir/graph/Value.h"
#include "lir/graph/Debug.h"

#include <format>

using namespace lir;

void DebugFile::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << '!' << m_id;
    } else if (policy == PrintPolicy::Def) {
        os << std::format("!{} = file {{ path: {}, file: {} }}\n", 
                          m_id, m_path, m_file);
    }
}

void DebugLoc::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << '!' << m_id;
    } else if (policy == PrintPolicy::Def) {
        os << std::format("!{} = loc {{ file: {}, line: {}, col: {} }}\n", 
                          m_id, m_file->id(), m_line, m_col);
    }
}

DebugBuilder::DebugBuilder(CFG& graph) : m_graph(graph), 
                                         m_symbols(graph.m_debug) {}

DebugFile* DebugBuilder::build_file(const std::string& path, 
                                    const std::string& file) {
    uint32_t fid = 0;
    for (DebugNode* node : m_symbols) {
        if (dynamic_cast<DebugFile*>(node))
            fid++;
    }

    DebugFile* node = new DebugFile(m_symbols.size(), fid, path, file);
    assert(node);

    m_symbols.push_back(node);
    return node;
}

DebugLoc* DebugBuilder::build_loc(DebugFile* file, uint32_t line, uint32_t col) {
    DebugLoc* node = new DebugLoc(m_symbols.size(), file, line, col);
    assert(node);

    m_symbols.push_back(node);
    return node;
}
