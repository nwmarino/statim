#include "siir/cfg.hpp"
#include "siir/global.hpp"
#include "siir/type.hpp"

using namespace stm;
using namespace stm::siir;

Global::Global(CFG& cfg, const Type* type, LinkageType linkage, bool read_only, 
               const std::string& name, Constant* init)
    : Constant({ init }, PointerType::get(cfg, type)), m_linkage(linkage),
      m_read_only(read_only), m_name(name), m_init(init) {

    cfg.add_global(this);
}
