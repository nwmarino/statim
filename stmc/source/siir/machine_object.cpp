#include "siir/machine_object.hpp"

using namespace stm;
using namespace stm::siir;

MachineObject::~MachineObject() {
    for (auto [name, function] : m_functions) {
        delete function;
    }

    m_functions.clear();
}
