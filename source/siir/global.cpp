#include "siir/global.hpp"

using namespace stm;
using namespace stm::siir;

Global::Global(const Type* type, LinkageTypes linkage, bool read_only, 
               Constant* init, CFG* parent, const std::string& name)
    : Constant({ init }, type, name), m_linkage(linkage), m_read_only(read_only),
      m_init(init), m_parent(parent) {}

Global* Global::create(const Type* type, LinkageTypes linkage, bool read_only, 
                       CFG* parent, Constant* init, const std::string& name) {
    return new Global(type, linkage, read_only, init, parent, name);
}
