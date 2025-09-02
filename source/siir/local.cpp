#include "siir/function.hpp"
#include "siir/type.hpp"
#include "siir/local.hpp"

using namespace stm;
using namespace stm::siir;

Local::Local(CFG& cfg, const Type* type, u32 align, const std::string& name, 
             Function* parent)
    : Value(PointerType::get(cfg, type)), m_alloc_type(type), m_align(align),
	  m_name(name), m_parent(parent) {

	if (parent)
		parent->add_local(this);
}
