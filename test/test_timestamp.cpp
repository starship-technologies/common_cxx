#include <gtest/gtest.h>
#include "common/common_timestamp.hpp"

TEST(system, now) {
    common::timestamp now = common::timestamp::now();
    const auto year2021 = common::timestamp{(2021-1970)*365*24*60*60, 0};
    EXPECT_GT(now, year2021);
}

