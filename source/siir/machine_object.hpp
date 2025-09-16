#ifndef STATIM_SIIR_MACHINE_OBJECT_H_
#define STATIM_SIIR_MACHINE_OBJECT_H_

#include "siir/target.hpp"

namespace stm::siir {

class MachineObject final {
    Target* m_target;

public:
    /// Return the target that this machine object was compiled for.
    const Target* get_target() const { return m_target; }
    Target* get_target() { return m_target; }
};

} // namespace stm::siir

#endif // STATIM_SIIR_MACHINE_OBJECT_H_
