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

/// Represents potentially named, constant data. Can exist at either the global
/// level or within a function pool. 
class MachineData final {
public:
    using Data = std::vector<MachineConstant>;

private:
    std::string m_name;
    Data m_data;
    bool m_pub;
    bool m_readonly;

public:
    MachineData(const std::string &name, const Data &data, bool pub = false, 
                bool readonly = true)
      : m_name(name), m_data(data), m_pub(pub), m_readonly(readonly) {}

    /// Returns the name of this data, if it has one.
    const std::string &get_name() const { return m_name; }

    /// Test if this data has a name.
    bool has_name() const { return !m_name.empty(); }

    const Data &get_data() const { return m_data; }
    Data &get_data() { return m_data; }

    inline bool is_public() const { return m_pub; }

    inline bool is_readonly() const { return m_readonly; }
};

} // namespace lir

#endif // LIR_MACHINE_DATA_H_
