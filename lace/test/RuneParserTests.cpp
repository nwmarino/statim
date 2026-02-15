//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"

#include "gtest/gtest.h"

namespace lace::test {

class RuneParserTests : public ::testing::Test {
protected:
    AST* unit;

    void SetUp() override {
        unit = nullptr;
    }

    void TearDown() override {
        if (unit) { 
            delete unit;
            unit = nullptr;
        }
    }
};

} // namespace lace::test
