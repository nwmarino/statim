//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_MACHINE_OBJECT_H_
#define LIR_MACHINE_OBJECT_H_

#include "lir/machine/Machine.hpp"
#include "lir/machine/MachineData.hpp"
#include "lir/machine/MachineOperand.hpp"

#include <string>
#include <unordered_map>

namespace lir {

class MachineObject final {
public:
    using Globals = std::unordered_map<std::string, MachineData*>;
    using Functions = std::unordered_map<std::string, MachineFunction*>;

private:
    const Machine &m_mach;

    Globals m_globals = {};
    Functions m_functions = {};

public:
    MachineObject(const Machine &mach) : m_mach(mach) {}

    ~MachineObject();

    MachineObject(const MachineObject&) = delete;
    void operator=(const MachineObject&) = delete;

    MachineObject(MachineObject&&) noexcept = delete;
    void operator=(MachineObject&&) noexcept = delete;

    const Machine &get_machine() const { return m_mach; }

    const Globals &get_globals() const { return m_globals; }
    Globals &get_globals() { return m_globals; }

    /// Returns the number of globals in this object.
    uint32_t num_globals() const { return m_globals.size(); }

    /// Returns the global in this object with the given |name|, if it exists.
    const MachineData *get_global(const std::string &name) const;
    MachineData *get_global(const std::string &name) {
        return const_cast<MachineData*>(
            static_cast<const MachineObject*>(this)->get_global(name));
    }

    const Functions &get_functions() const { return m_functions; }
    Functions &get_functions() { return m_functions; }

    /// Returns the number of functions in this object.
    uint32_t num_functions() const { return m_functions.size(); }

    /// Returns the function in this object with the given |name|, if it exists.
    const MachineFunction *get_function(const std::string &name) const;
    MachineFunction *get_function(const std::string &name) {
        return const_cast<MachineFunction*>(
            static_cast<const MachineObject*>(this)->get_function(name));
    }

    /// Test if this object is empty, i.e. contains no globals or functions.
    bool empty() const { return m_globals.empty() && m_functions.empty(); }
};

} // namespace lir

#endif // LIR_MACHINE_OBJECT_H_
