//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/CFG.h"
#include "lir/graph/Function.h"
#include "lir/graph/Local.h"
#include "lir/graph/Type.h"
#include "lir/graph/Value.h"

#include <format>

using namespace lir;

Local *Local::create(CFG &cfg, Type *type, const std::string &name, 
					 Function *parent, uint32_t align) {
	if (align == 0)
		align = (cfg.get_machine().get_type_align(type) / 8);

	Local *local = new Local(PointerType::get(cfg, type), nullptr, name, align);
	assert(local);

	if (parent)
		parent->add_local(local);

	return local;
}

void Local::detach() {
	assert(has_parent() && "local does not belong to a function!");

	m_parent->remove_local(this);
}

void Local::print(std::ostream &os, PrintPolicy policy) const {
	if (policy == PrintPolicy::Use) {
		os << std::format("%{}: {}", m_name, m_type->to_string());
	} else if (policy == PrintPolicy::Def) {
		os << std::format("%{} := local <{}> [{}]\n", 
			m_name, get_allocated_type()->to_string(), m_align);
	}
}
