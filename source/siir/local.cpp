#include "siir/local.hpp"

using namespace stm;
using namespace stm::siir;

Local::Local(const Type* alloc_type, u32 align, const Type* type, 
             const std::string& name, Function* parent)
    : Value(type, name), m_alloc_type(type), m_align(align), m_parent(parent) {}

Local* Local::create(const Type* alloc_type, u32 align, const Type* type,
                     const std::string& name, Function* parent) {
    return new Local(alloc_type, align, type, name, parent);
}
