#include "siir/use.hpp"
#include <algorithm>
#include "siir/value.hpp"

using namespace stm;
using namespace stm::siir;

Value::Value(const Type* type, const std::string& name) 
    : m_type(type), m_name(name), m_uses() {}

Value::~Value() {
    //for (auto use : m_uses) delete use;
    m_uses.clear();
}

void Value::add_use(Use* use) {
    m_uses.push_back(use);
}

void Value::del_use(Use* use) {
    auto it = std::find(m_uses.begin(), m_uses.end(), use);
    if (it != m_uses.end())
        m_uses.erase(it);
}

void Value::replace_all_uses_with(Value* value) {
    for (auto use : m_uses) {
        /// TODO: ...
    }   
}
