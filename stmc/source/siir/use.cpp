#include "siir/use.hpp"
#include "siir/instruction.hpp"

using namespace stm;
using namespace siir;

Use::Use(Value* value, User* user) : m_value(value), m_user(user) {
    assert(value);
    assert(user);

    if (auto* phi_op = dynamic_cast<PhiOperand*>(value))
        phi_op->get_value()->add_use(this);
    else
        value->add_use(this);
}

Use::~Use() {
    m_value->del_use(this);
    m_value = nullptr;
    m_user = nullptr;
}
