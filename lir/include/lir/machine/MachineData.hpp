//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_MACHINE_DATA_H_
#define LIR_MACHINE_DATA_H_

#include "lir/machine/MachineConstant.hpp"

#include <string>
#include <vector>

namespace lir {

class ConstantPool;

/// Represents potentially named, constant data. Can exist at either the global
/// level or within a function pool. 
class MachineData final {
    const ConstantPool* m_pool;
    std::string m_name;
    std::vector<MachineConstant> m_data;
    bool m_global;
    bool m_readonly;

public:
    MachineData(const std::string& name, const std::vector<MachineConstant>& data, 
                bool global = false, bool readonly = true, ConstantPool* pool = nullptr)
      : m_name(name), m_data(data), m_global(global), m_readonly(readonly), m_pool(pool) {}

    const ConstantPool* pool() const { return m_pool; }

    bool hasPool() const { return m_pool != nullptr; }

    /// Returns the name of this data.
    const std::string& name() const { return m_name; }

    const std::vector<MachineConstant>& data() const { return m_data; }
    std::vector<MachineConstant>& data() { return m_data; }

    bool isGlobal() const { return m_global; }

    inline bool isReadonly() const { return m_readonly; }
};

} // namespace lir

#endif // LIR_MACHINE_DATA_H_
