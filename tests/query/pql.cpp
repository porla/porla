#include <gtest/gtest.h>
#include <libtorrent/torrent_status.hpp>

#include "../../src/query/pql.hpp"

using porla::Query::PQL;

class PqlParameterizedTestFixture :public ::testing::TestWithParam<std::string>{};

TEST_P(PqlParameterizedTestFixture, Parses_Valid_Queries)
{
    std::string query = GetParam();
    ASSERT_TRUE(PQL::Parse(query) != nullptr);
}

INSTANTIATE_TEST_SUITE_P(
    PQLParserTests,
    PqlParameterizedTestFixture,
    ::testing::Values(
        "age > 1d",
        "category = \"some-category\"",
        "dl > 1mbps",
        "name contains \"foo\"",
        "is:downloading",
        "is:finished",
        "is:paused",
        "is:seeding",
        "is:seeding and age > 1w",
        "size > 1gb",
        "size < 1gb or age > 3h",
        "size > 1gb and tags contains \"foo\"",
        "ul < 1mbps"
    ));

TEST(porla_Query_PQL, Filter_Name_Contains)
{
    libtorrent::torrent_status status;
    status.name = "i am ubuntu 123 åäö";

    EXPECT_EQ(PQL::Parse("name contains \"ubuntu\"")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("name contains \"debian\"")->Includes(status), false);
}

TEST(porla_Query_PQL, Filter_Age)
{
    libtorrent::torrent_status status;
    status.added_time = time(nullptr) - 3601;

    EXPECT_EQ(PQL::Parse("age >= 1h")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("age <  3w")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("age >  1h")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("age > 27m")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("age <  1h")->Includes(status), false);
    EXPECT_EQ(PQL::Parse("age <= 1h")->Includes(status), false);
}

TEST(porla_Query_PQL, Filter_DL)
{
    libtorrent::torrent_status status;
    status.download_payload_rate = 2 * 1024 * 1024; // 2 mbps

    EXPECT_EQ(PQL::Parse("dl >= 1")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("dl <  10mbps")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("dl >  500kbps")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("dl > 103")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("dl <  1mbps")->Includes(status), false);
    EXPECT_EQ(PQL::Parse("dl <= 346kbps")->Includes(status), false);
}
