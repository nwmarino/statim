//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"

#include "gtest/gtest.h"

namespace lace::test {

class CodegenTests : public ::testing::Test {
protected:
    AST* ast;

    void SetUp() override {
        ast = nullptr;
    }

    void TearDown() override {
        if (ast) { 
            delete ast;
            ast = nullptr;
        }
    }
};

} // namespace lace::test
