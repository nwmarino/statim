#ifndef STATIM_SIIR_MACHINE_OBJECT_H_
#define STATIM_SIIR_MACHINE_OBJECT_H_

#include "siir/machine_function.hpp"
#include "siir/target.hpp"

#include <string>
#include <unordered_map>

namespace stm::siir {

class MachineObject final {
    const Target* m_target;
    std::unordered_map<std::string, MachineFunction*> m_functions;

public:
    /// Create a new machine object for the given target.
    MachineObject(const Target* target) : m_target(target) {}

    MachineObject(const MachineObject&) = delete;
    MachineObject& operator = (const MachineObject&) = delete;

    ~MachineObject();

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
