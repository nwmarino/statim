//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_CONTEXT_H_
#define LACE_CONTEXT_H_

#include "lace/core/Options.h"

#include <unordered_map>
#include <vector>

namespace lace {

class Rib;

class Context final {
    Options m_options = {};
    std::unordered_map<std::string, Rib*> m_ribs = {};

public:
    Context(const Options& options) : m_options(options) {}

    ~Context();

    Context(const Context&) = delete;
    void operator=(const Context&) = delete;

    Context(Context&&) noexcept = delete;
    void operator=(Context&&) noexcept = delete;

    const Options& options() const { return m_options; }
    Options& options() { return m_options; }
    
    const std::unordered_map<std::string, Rib*>& ribs() const { return m_ribs; }
    std::unordered_map<std::string, Rib*>& ribs() { return m_ribs; }

    bool add_rib(Rib* rib);

    Rib* get_rib(const std::string& name) const;
};

} // namespace lace

#endif //LACE_CONTEXT_H_
