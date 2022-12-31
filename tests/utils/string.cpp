#include <gtest/gtest.h>

#include "../../src/utils/string.hpp"

using porla::Utils::String;

TEST(String, Split_ForHappyPath_ReturnsCorrectItems)
{
    const auto result = String::Split("foo,bar", ",");
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0), "foo");
    EXPECT_EQ(result.at(1), "bar");
}

TEST(String, Split_ForComplexPath_ReturnsCorrectItems)
{
    const auto result = String::Split("foo,,bar,,baz,", ",");
    EXPECT_EQ(result.size(), 6);
    EXPECT_EQ(result.at(0), "foo");
    EXPECT_EQ(result.at(1), "");
    EXPECT_EQ(result.at(2), "bar");
    EXPECT_EQ(result.at(3), "");
    EXPECT_EQ(result.at(4), "baz");
    EXPECT_EQ(result.at(5), "");
}
