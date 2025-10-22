#include "siir/use.hpp"
#include "siir/value.hpp"

#include <algorithm>

using namespace stm;
using namespace stm::siir;

void Value::add_use(Use* use) {
    m_uses.push_back(use);
}

void Value::del_use(Use* use) {
    auto it = std::find(m_uses.begin(), m_uses.end(), use);
    if (it != m_uses.end())
        m_uses.erase(it);
}

void Value::replace_all_uses_with(Value* value) {
    std::vector<Use*> uses_copy = m_uses;
    for (Use* use : uses_copy) {
        use->set_value(value);
    }
}
