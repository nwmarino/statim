//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/CFG.hpp"
#include "lir/graph/Global.hpp"
#include "lir/graph/Type.hpp"

#include <format>

using namespace lir;

Global *Global::create(CFG &cfg, Type *type, LinkageType linkage, 
                       const std::string &name, bool mut, Constant *init) {
	Global *global = new Global(
		lir::PointerType::get(cfg, type), 
		&cfg, 
		linkage,  
		name, 
		mut,
		init
	);
	assert(global);

	cfg.add_global(global);
	return global;
}

void Global::print(std::ostream &os, PrintPolicy policy) const {
	if (policy == PrintPolicy::Use) {
		os << std::format("@{}: {}", m_name, m_type->to_string());
    } else if (policy == PrintPolicy::Def) {
		os << std::format("@{} := ", get_name());

		if (has_linkage(LinkageType::Public))
			os << "pub ";

		if (is_mutable())
			os << "mut ";
		
		os << m_type->to_string();

		if (has_initializer()) {
			os << ' ';
			get_initializer()->print(os, PrintPolicy::Use);
		}
	}
}
