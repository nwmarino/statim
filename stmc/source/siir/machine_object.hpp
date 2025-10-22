#ifndef STATIM_SIIR_MACHINE_OBJECT_H_
#define STATIM_SIIR_MACHINE_OBJECT_H_

#include "siir/machine_function.hpp"
#include "siir/target.hpp"

#include <string>
#include <unordered_map>

namespace stm::siir {

class MachineObject final {
    const Target* m_target;
    const CFG* m_cfg;
    std::unordered_map<std::string, MachineFunction*> m_functions;

public:
    /// Create a new machine object for the given target.
    MachineObject(const CFG* cfg, const Target* target) 
        : m_cfg(cfg), m_target(target) {}

    MachineObject(const MachineObject&) = delete;
    MachineObject& operator = (const MachineObject&) = delete;

    ~MachineObject();

    /// Returns the SIIR control flow graph this machine object was lowered
    /// from.
    const CFG* get_graph() const { return m_cfg; }

    /// Return the target that this machine object was compiled for.
    const Target* get_target() const { return m_target; }

    const std::unordered_map<std::string, MachineFunction*>& functions() const {
        return m_functions;
    }

    std::unordered_map<std::string, MachineFunction*>& functions() {
        return m_functions;
    }
};

} // namespace stm::siir

#endif // STATIM_SIIR_MACHINE_OBJECT_H_
