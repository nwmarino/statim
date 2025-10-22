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

void Local::detach_from_parent() {
	assert(m_parent && "local does not belong to a function");
	m_parent->remove_local(this);
}
