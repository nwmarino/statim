//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Expr.h"
#include "lace/tree/Rune.h"

using namespace lace;

Rune::~Rune() {
    for (Expr* arg : m_args)
        delete arg;

    m_args.clear();
}
