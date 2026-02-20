//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"

#include "gtest/gtest.h"

namespace lace::test {

class ExprParserTests : public ::testing::Test {
protected:
    AST* ast;

    void SetUp() override {
        ast = nullptr;
    }

    void TearDown() override {
        if (ast)
            delete ast;
            
        ast = nullptr;
    }
};

} // namespace lace::test
