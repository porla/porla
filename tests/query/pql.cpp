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
    PqlParserTests,
    PqlParameterizedTestFixture,
    ::testing::Values(
        "age > 1d",
        "category = \"some-category\"",
        "download_rate > 1mbps",
        "name contains \"foo\"",
        "is:downloading",
        "not is:downloading",
        "is:downloading and not is:paused",
        "is:finished",
        "is:paused",
        "is:seeding",
        "is:seeding and age > 1w",
        "progress <= 0.8",
        "save_path = \"/dl\"",
        "size > 1gb",
        "size < 1gb or age > 3h",
        "size > 1gb and tags contains \"foo\"",
        "upload_rate < 1mbps"
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

TEST(porla_Query_PQL, Filter_DownloadRate)
{
    libtorrent::torrent_status status;
    status.download_rate = 2 * 1024 * 1024; // 2 mbps

    EXPECT_EQ(PQL::Parse("download_rate >= 1")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("download_rate <  10mbps")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("download_rate >  500kbps")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("download_rate > 103")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("download_rate <  1mbps")->Includes(status), false);
    EXPECT_EQ(PQL::Parse("download_rate <= 346kbps")->Includes(status), false);
}

TEST(porla_Query_PQL, Filter_Flags)
{
    libtorrent::torrent_status status;
    status.state = lt::torrent_status::seeding;

    EXPECT_EQ(PQL::Parse("is:seeding")->Includes(status), true);
    EXPECT_EQ(PQL::Parse("not is:seeding")->Includes(status), false);
    EXPECT_EQ(PQL::Parse("is:seeding and not is:paused")->Includes(status), true);
}
