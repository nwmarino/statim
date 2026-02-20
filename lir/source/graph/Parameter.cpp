//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/Function.h"
#include "lir/graph/Value.h"
#include "lir/graph/BasicBlock.h"

#include <format>

using namespace lir;

Parameter *Parameter::create(Type *type, const std::string &name, Trait trait, Function *parent) {
    Parameter *param = new Parameter(type, parent, name, trait);
    assert(param);
    
    if (parent)
        parent->add_param(param);

    return param;
}

uint32_t Parameter::get_index() const {
    assert(has_parent() && "parameter does not belong to a function!");

    uint32_t i = 0;
    for (const Parameter *param : get_parent()->get_params()) {
        if (param == this)
            return i;

        i++;
    }

    assert(false && "parameter is missing from parent function!");
}

void Parameter::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        assert(is_named() && "cannot use unnamed argument!");

        os << std::format("{}: {}", get_name(), get_type()->to_string());
    } else if (policy == PrintPolicy::Def) {
        if (is_named())
            os << std::format("{}: ", get_name());

        switch (getTrait()) {
            case Trait::None:
                break;
            case Trait::ARet:
                os << "aret ";
                break;
            case Trait::Byval:
                os << "byval ";
                break;
        }

        os << get_type()->to_string();
    }
}
