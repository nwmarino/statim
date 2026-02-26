//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/NameResolution.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

using namespace lace;

Type* NameResolution::resolve_type(Type* type) const {
    if (auto deferred = dynamic_cast<DeferredType*>(type)) {
        NamedDefn* named_defn = m_scope->get(deferred->name()); 
        if (!named_defn)
            return nullptr;

        TypeDefn* type_defn = dynamic_cast<TypeDefn*>(named_defn);
        if (!type_defn)
            return nullptr;

        return type_defn->type();
    } else if (auto enumeration = dynamic_cast<EnumType*>(type)) {
        Type* underlying = resolve_type(enumeration->underlying());
        if (underlying != enumeration->underlying())
            enumeration->set_underlying(underlying);

        return enumeration;
    } else if (auto func = dynamic_cast<FunctionType*>(type)) {
        Type* result = resolve_type(func->result());
        if (!result)
            return nullptr;

        std::vector<Type*> params = {};
        params.reserve(func->num_params());

        for (Type* param : func->params()) {
            Type* res = resolve_type(param);
            if (!res)
                return nullptr;

            params.push_back(res);
        }

        return FunctionType::get(*m_ast, result, params);
    } else if (auto ptr = dynamic_cast<PointerType*>(type)) {
        Type* pointee = resolve_type(ptr->pointee());
        if (!pointee)
            return nullptr;

        if (pointee != ptr->pointee())
            ptr->set_pointee(pointee);

        return ptr;
    }

    return type;
}
