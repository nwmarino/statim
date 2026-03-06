//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Rib.h"

#include "gtest/gtest.h"

namespace lace::test {

class RuneParserTests : public ::testing::Test {
protected:
    Rib* rib;

    void SetUp() override {
        rib = nullptr;
    }

    void TearDown() override {
        if (rib)
            delete rib;

        rib = nullptr;
    }
};

} // namespace lace::test
