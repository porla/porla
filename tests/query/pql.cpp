#include <gtest/gtest.h>

#include <ctime>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <libtorrent/load_torrent.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../../src/query/pql.hpp"
#include "../../src/torrentclientdata.hpp"
#include "../../src/utils/hex.hpp"

using porla::Query::PQL;
using porla::Query::QueryContext;
using porla::Query::QueryError;

namespace
{
    constexpr std::int64_t KiB = 1024;
    constexpr std::int64_t MiB = 1024 * KiB;
    constexpr std::int64_t GiB = 1024 * MiB;

    constexpr std::string_view kV1 = "3f9aac158c7de8dfcab171ea58a17aabdf7fbc93";
    constexpr std::string_view kV2 = "a1b2c3d4e5f60718293a4b5c6d7e8f90112233445566778899aabbccddeeff00";

    std::time_t Local(int y, int mo, int d, int h = 0, int mi = 0, int s = 0)
    {
        std::tm tm{};
        tm.tm_year  = y - 1900;
        tm.tm_mon   = mo - 1;
        tm.tm_mday  = d;
        tm.tm_hour  = h;
        tm.tm_min   = mi;
        tm.tm_sec   = s;
        tm.tm_isdst = -1;

        return std::mktime(&tm);
    }

    const std::time_t kNow = Local(2026, 1, 2, 12, 0);

    template<typename THash>
    THash FromHex(std::string_view hex)
    {
        THash h;
        porla::Utils::FromHex(hex, reinterpret_cast<char*>(h.data()), h.size());
        return h;
    }

    std::shared_ptr<const lt::torrent_info> MakeTorrentInfo(bool is_private)
    {
        const std::string buf =
            "d4:infod6:lengthi1e4:name1:a12:piece lengthi16384e6:pieces20:"
            + std::string(20, '\0')
            + (is_private ? "7:privatei1e" : "")
            + "ee";

        return lt::load_torrent_buffer(
            lt::span<char const>(buf.data(), static_cast<std::ptrdiff_t>(buf.size()))).ti;
    }

    struct Torrent
    {
        lt::torrent_status                      status;
        porla::TorrentClientData                client_data;
        bool                                    with_client_data = true;
        std::time_t                             now              = kNow;
        std::shared_ptr<const lt::torrent_info> ti;

        [[nodiscard]] QueryContext Context() const
        {
            return { status, with_client_data ? &client_data : nullptr, now };
        }
    };

    using Setup = std::function<void(Torrent&)>;

    struct Case
    {
        std::string name;
        std::string query;
        bool        expected;
        Setup       setup = nullptr;
    };

    void PrintTo(const Case& c, std::ostream* os) { *os << '"' << c.query << '"'; }

    std::vector<Case> With(const Setup& base, std::vector<Case> cases)
    {
        for (auto& c : cases)
        {
            c.setup = [base, own = c.setup](Torrent& t)
            {
                base(t);
                if (own) { own(t); }
            };
        }

        return cases;
    }

    const auto CaseName = [](const auto& info) { return info.param.name; };
}

class PqlMatch : public ::testing::TestWithParam<Case> {};

TEST_P(PqlMatch, Evaluates)
{
    const auto& c = GetParam();

    Torrent t;
    if (c.setup) { c.setup(t); }

    EXPECT_EQ(PQL::Parse(c.query)(t.Context()), c.expected) << c.query;
}

TEST_P(PqlMatch, NeverThrowsOnEmptyTorrent)
{
    Torrent t;
    t.with_client_data = false;

    const auto filter = PQL::Parse(GetParam().query);
    EXPECT_NO_THROW(filter(t.Context())) << GetParam().query;
}

// --- syntax ------------------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(Syntax, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t) { t.status.name = "ubuntu-24.04-desktop-amd64.iso"; },
    {
        { "empty_matches_all",       "",                              true  },
        { "implicit_and",            "ubuntu desktop",                true  },
        { "implicit_and_miss",       "ubuntu debian",                 false },
        { "and_keyword",             "ubuntu AND desktop",            true  },
        { "and_symbol",              "ubuntu && desktop",             true  },
        { "or_keyword",              "debian OR ubuntu",              true  },
        { "or_symbol",               "debian || ubuntu",              true  },
        { "lowercase_or_is_text",    "debian or ubuntu",              false },
        { "not_dash",                "-debian",                       true  },
        { "not_bang",                "!ubuntu",                       false },
        { "not_keyword",             "NOT ubuntu",                    false },
        { "double_not_bang",         "!!ubuntu",                      true  },
        { "double_not_dash",         "--ubuntu",                      true  },
        { "group",                   "(debian OR ubuntu) desktop",    true  },
        { "and_binds_tighter",       "debian OR ubuntu server",       false },
        { "word_with_dash_and_dot",  "24.04-desktop",                 true  },
        { "dash_digit_is_not",       "-3f",                           true  },
        { "ascii_case_insensitive",  "UBUNTU",                        true  },
        { "phrase",                  "\"24.04 desktop\"",             true,  [](Torrent& t) { t.status.name = "Ubuntu 24.04 Desktop"; } },
        { "phrase_order",            "\"desktop 24.04\"",             false, [](Torrent& t) { t.status.name = "Ubuntu 24.04 Desktop"; } },
        { "escaped_quote",           R"("say \"hi\"")",               true,  [](Torrent& t) { t.status.name = R"(say "hi")"; } },
        { "utf8_exact",              "Ångström",                      true,  [](Torrent& t) { t.status.name = "Ångström"; } },
        { "utf8_no_case_folding",    "ångström",                      false, [](Torrent& t) { t.status.name = "Ångström"; } },
    })), CaseName);

// --- text fields -------------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(Text, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t)
    {
        t.status.name            = "ubuntu-24.04-desktop-amd64.iso";
        t.status.save_path       = "/data/downloads/linux";
        t.status.current_tracker = "udp://tracker.example.org:1337/announce";
        t.client_data.category   = "Linux";
        t.client_data.tags       = { "iso", "lts" };
    },
    {
        { "name_contains",               "name:desktop",                           true  },
        { "name_contains_case",          "name:DESKTOP",                           true  },
        { "name_miss",                   "name:debian",                            false },
        { "name_eq",                     "name:=UBUNTU-24.04-desktop-amd64.iso",   true  },
        { "name_eq_partial",             "name:=ubuntu",                           false },
        { "name_glob",                   "name:ubuntu*.iso",                       true  },
        { "name_glob_anchored",          "name:*desktop",                          false },
        { "name_list",                   "name:debian,desktop",                    true  },
        { "name_quoted_no_split",        "name:\"debian,desktop\"",                false },
        { "path",                        "path:/downloads/",                       true  },
        { "category_contains",           "category:lin",                           true  },
        { "category_eq",                 "category:=linux",                        true  },
        { "category_no_client_data",     "category:linux",                         false, [](Torrent& t) { t.with_client_data = false; } },
        { "category_unset",              "category:linux",                         false, [](Torrent& t) { t.client_data.category.reset(); } },
        { "tag_exact",                   "tag:iso",                                true  },
        { "tag_case",                    "tag:ISO",                                true  },
        { "tag_no_substring",            "tag:is",                                 false },
        { "tag_glob",                    "tag:l*",                                 true  },
        { "tag_any",                     "tag:foo,lts",                            true  },
        { "tag_all",                     "tag:iso tag:foo",                        false },
        { "tag_no_client_data",          "tag:iso",                                false, [](Torrent& t) { t.with_client_data = false; } },
        { "tracker",                     "tracker:example.org",                    true  }
    })), CaseName);

// --- hashes ------------------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(Hash, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t)
    {
        t.status.name           = "ubuntu";
        t.status.info_hashes.v1 = FromHex<lt::sha1_hash>(kV1);
        t.status.info_hashes.v2 = FromHex<lt::sha256_hash>(kV2);
    },
    {
        { "v1_prefix",               "hash:3f9a",                     true  },
        { "v2_prefix",               "hash:a1b2",                     true  },
        { "v1_full",                 "hash:" + std::string(kV1),      true  },
        { "v2_full",                 "hash:" + std::string(kV2),      true  },
        { "uppercase",               "hash:3F9AAC15",                 true  },
        { "miss",                    "hash:0000",                     false },
        { "list",                    "hash:0000,3f9a",                true  },
        { "v2_absent",               "hash:a1b2",                     false, [](Torrent& t) { t.status.info_hashes.v2.clear(); } },
        { "free_text_digit_first",   "3f9aac15",                      true  },
        { "free_text_letter_first",  "a1b2c3d4",                      true  },
        { "free_text_too_short",     "3f9a",                          false },
        { "free_text_negated",       "-3f9aac15",                     false },
    })), CaseName);

// --- sizes and rates ---------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(Size, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t)
    {
        t.status.total_wanted      = 3 * GiB / 2;
        t.status.total_wanted_done = 1 * GiB;
        t.status.total_done        = 1 * GiB;
        t.status.all_time_upload   = 2 * MiB;
    },
    {
        { "gt",             "size:>1gb",        true  },
        { "lt",             "size:<1gb",        false },
        { "float_gte",      "size:>=1.5gb",     true  },
        { "float_gt",       "size:>1.5gib",     false },
        { "gib",            "size:>1gib",       true  },
        { "uppercase_unit", "size:>1GB",        true  },
        { "bytes_eq",       "size:1610612736",  true  },
        { "b_suffix",       "size:>10b",        true  },
        { "range",          "size:1gb..2gb",    true  },
        { "range_miss",     "size:2gb..3gb",    false },
        { "downloaded",     "downloaded:>=1gb", true  },
        { "uploaded",       "uploaded:>1mb",    true  },
        { "remaining",      "remaining:512mib", true  },
        { "gb_is_decimal",  "size:>1.6gb",      true },
        { "gib_is_binary",  "size:>1.6gib",     false }
    })), CaseName);

INSTANTIATE_TEST_SUITE_P(Rate, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t)
    {
        t.status.download_payload_rate = 2 * MiB;
        t.status.upload_payload_rate   = 0;
    },
    {
        { "dl_gt",               "dl:>500kb",            true  },
        { "dl_per_second",       "dl:>1mb/s",            true  },
        { "dl_alias",            "download_rate:>1mb",   true  },
        { "dl_lt",               "dl:<1mb",              false },
        { "ul_zero",             "ul:0",                 true  },
        { "ul_alias",            "upload_rate:0",        true  },
    })), CaseName);

// --- durations ---------------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(Duration, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t)
    {
        t.status.added_time        = kNow - 3601;
        t.status.active_duration   = std::chrono::seconds(86400);
        t.status.seeding_duration  = std::chrono::seconds(8 * 86400);
        t.status.finished_duration = std::chrono::seconds(0);
    },
    {
        { "age_gt",                  "age:>1h",              true  },
        { "age_lt",                  "age:<1h",              false },
        { "age_bare_seconds",        "age:>3600",            true  },
        { "age_minutes",             "age:>59m",             true  },
        { "age_days",                "age:<5d",              true  },
        { "age_months",              "age:<1mo",             true  },
        { "age_years",               "age:<1y",              true  },
        { "age_range",               "age:1h..2h",           true  },
        { "active_time",             "active_time:>=1d",     true  },
        { "seed_time",               "seed_time:>1w",        true  },
        { "finished_time",           "finished_time:0",      true  },
        { "eta",                     "eta:<1h",              true,  [](Torrent& t) { t.status.total_wanted = 101 * MiB; t.status.total_wanted_done = 1 * MiB; t.status.download_payload_rate = 1 * MiB; } },
        { "eta_unknown_never_match", "eta:>0",               false, [](Torrent& t) { t.status.total_wanted = 101 * MiB; t.status.download_payload_rate = 0; } },
        { "eta_unknown_negated",     "-eta:<1h",             true,  [](Torrent& t) { t.status.total_wanted = 101 * MiB; t.status.download_payload_rate = 0; } },
    })), CaseName);

// --- percent and numbers -----------------------------------------------------

INSTANTIATE_TEST_SUITE_P(Percent, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t) { t.status.progress_ppm = 750000; t.status.progress = 0.75f; },
    {
        { "gt",                  "progress:>50",         true  },
        { "percent_sign",        "progress:>50%",        true  },
        { "lt",                  "progress:<50",         false },
        { "range",               "progress:25..75",      true  },
        { "exact_100",           "progress:100",         true,  [](Torrent& t) { t.status.progress_ppm = 1000000; t.status.progress = 1.f; } },
        { "float",               "progress:>=99.5",      true,  [](Torrent& t) { t.status.progress_ppm = 995000;  t.status.progress = .995f; } },
    })), CaseName);

INSTANTIATE_TEST_SUITE_P(Number, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t)
    {
        t.status.all_time_upload   = 2 * GiB;
        t.status.all_time_download = 1 * GiB;
        t.status.total_done        = 1 * GiB;
        t.status.num_seeds         = 10;
        t.status.num_peers         = 0;
        t.status.queue_position    = lt::queue_position_t{ 3 };
    },
    {
        { "ratio_gte",           "ratio:>=2",            true  },
        { "ratio_float",         "ratio:<2.5",           true  },
        { "ratio_lt",            "ratio:<1",             false },
        { "seeds",               "seeds:>=10",           true  },
        { "peers",               "peers:0",              true  },
        { "queue",               "queue:3",              true  },
    })), CaseName);

// --- dates (server local time; run under a non-UTC TZ) -----------------------

INSTANTIATE_TEST_SUITE_P(Date, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t)
    {
        t.status.added_time     = Local(2026, 1, 1, 12, 0);
        t.status.completed_time = 0;
    },
    {
        { "day",                 "added:2026-01-01",                 true  },
        { "other_day",           "added:2025-12-31",                 false },
        { "gt_excludes_day",     "added:>2026-01-01",                false },
        { "gte_includes_day",    "added:>=2026-01-01",               true  },
        { "lt_next_day",         "added:<2026-01-02",                true  },
        { "lte_includes_day",    "added:<=2026-01-01",               true  },
        { "range_inclusive",     "added:2025-12-01..2026-01-01",     true  },
        { "minute",              "added:2026-01-01T12:00",           true  },
        { "gt_minute",           "added:>2026-01-01T12:00",          false },
        { "day_end_boundary",    "added:2026-01-01",                 true,  [](Torrent& t) { t.status.added_time = Local(2026, 1, 1, 23, 59, 59); } },
        { "next_day_boundary",   "added:2026-01-01",                 false, [](Torrent& t) { t.status.added_time = Local(2026, 1, 2, 0, 0, 0); } },
        { "never_completed",     "completed:>2000-01-01",            false },
        { "completed",           "completed:2026-01-01",             true,  [](Torrent& t) { t.status.completed_time = Local(2026, 1, 1, 18, 0); } },
    })), CaseName);

// --- is: and has: ------------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(Flags, PqlMatch, ::testing::Values(
    Case{ "downloading",             "is:downloading",   true,  [](Torrent& t) { t.status.state = lt::torrent_status::downloading; } },
    Case{ "seeding",                 "is:seeding",       true,  [](Torrent& t) { t.status.state = lt::torrent_status::seeding; } },
    Case{ "finished",                "is:finished",      true,  [](Torrent& t) { t.status.state = lt::torrent_status::finished; } },
    Case{ "paused",                  "is:paused",        true,  [](Torrent& t) { t.status.flags = lt::torrent_flags::paused; } },
    Case{ "queued",                  "is:queued",        true,  [](Torrent& t) { t.status.flags = lt::torrent_flags::paused | lt::torrent_flags::auto_managed; } },
    Case{ "queued_not_manual_pause", "is:queued",        false, [](Torrent& t) { t.status.flags = lt::torrent_flags::paused; } },
    Case{ "checking_files",          "is:checking",      true,  [](Torrent& t) { t.status.state = lt::torrent_status::checking_files; } },
    Case{ "checking_resume",         "is:checking",      true,  [](Torrent& t) { t.status.state = lt::torrent_status::checking_resume_data; } },
    Case{ "moving",                  "is:moving",        true,  [](Torrent& t) { t.status.moving_storage = true; } },
    Case{ "error",                   "is:error",         true,  [](Torrent& t) { t.status.errc = boost::system::errc::make_error_code(boost::system::errc::io_error); } },
    Case{ "no_error",                "is:error",         false },
    Case{ "private",                 "is:private",       true,  [](Torrent& t) { t.ti = MakeTorrentInfo(true);  t.status.torrent_file = t.ti; } },
    Case{ "public",                  "is:private",       false, [](Torrent& t) { t.ti = MakeTorrentInfo(false); t.status.torrent_file = t.ti; } },
    Case{ "private_no_metadata",     "is:private",       false },
    Case{ "stalled",                 "is:stalled",       true,  [](Torrent& t) { t.status.state = lt::torrent_status::downloading; t.status.download_payload_rate = 0; } },
    Case{ "not_stalled",             "is:stalled",       false, [](Torrent& t) { t.status.state = lt::torrent_status::downloading; t.status.download_payload_rate = 1; } },
    Case{ "active_upload",           "is:active",        true,  [](Torrent& t) { t.status.upload_payload_rate = 1; } },
    Case{ "inactive",                "is:active",        false },
    Case{ "is_list",                 "is:paused,seeding", true, [](Torrent& t) { t.status.state = lt::torrent_status::seeding; } },
    Case{ "has_category",            "has:category",     true,  [](Torrent& t) { t.client_data.category = "tv"; } },
    Case{ "has_category_unset",      "has:category",     false },
    Case{ "has_category_no_data",    "has:category",     false, [](Torrent& t) { t.with_client_data = false; } },
    Case{ "has_tags",                "has:tags",         true,  [](Torrent& t) { t.client_data.tags = { "a" }; } },
    Case{ "not_has_tags",            "-has:tags",        true  },
    Case{ "has_metadata",            "has:metadata",     true,  [](Torrent& t) { t.status.has_metadata = true; } },
    Case{ "has_error",               "has:error",        true,  [](Torrent& t) { t.status.errc = boost::system::errc::make_error_code(boost::system::errc::io_error); } }
), CaseName);

// --- rejected queries --------------------------------------------------------

struct Reject
{
    std::string name;
    std::string query;
    std::string message_contains = {};
};

void PrintTo(const Reject& r, std::ostream* os) { *os << '"' << r.query << '"'; }

class PqlRejects : public ::testing::TestWithParam<Reject> {};

TEST_P(PqlRejects, Throws)
{
    const auto& r = GetParam();

    try
    {
        PQL::Parse(r.query);
        FAIL() << "expected QueryError for: " << r.query;
    }
    catch (const QueryError& e)
    {
        EXPECT_NE(std::string(e.what()).find(r.message_contains), std::string::npos)
            << "message: " << e.what();
    }
}

INSTANTIATE_TEST_SUITE_P(Pql, PqlRejects, ::testing::Values(
    Reject{ "unknown_field",           "szie:1",                    "did you mean 'size'" },
    Reject{ "old_save_path",           "save_path:/x",              "did you mean 'path'" },
    Reject{ "old_tags",                "tags:foo",                  "did you mean 'tag'" },
    Reject{ "old_active_duration",     "active_duration:1h",        "did you mean 'active_time'" },
    Reject{ "unknown_is",              "is:bogus" },
    Reject{ "unknown_has",             "has:bogus" },
    Reject{ "unknown_size_unit",       "size:>1zb",                 "zb" },
    Reject{ "bits_unit_dropped",       "dl:>1mbps",                 "mbps" },
    Reject{ "duration_unit_on_size",   "size:>1h" },
    Reject{ "size_unit_on_duration",   "age:>1gb" },
    Reject{ "op_on_text",              "name:>abc" },
    Reject{ "op_on_tag",               "tag:>a" },
    Reject{ "op_on_flag",              "is:>paused" },
    Reject{ "text_for_size",           "size:abc" },
    Reject{ "list_on_size",            "size:1gb,2gb" },
    Reject{ "glob_on_size",            "size:1*" },
    Reject{ "range_reversed",          "size:2gb..1gb" },
    Reject{ "progress_over_100",       "progress:>101" },
    Reject{ "negative_number",         "progress:>-1" },
    Reject{ "bad_month",               "added:2026-13-01" },
    Reject{ "bad_day",                 "added:2026-02-30" },
    Reject{ "bad_date_format",         "added:01/01/2026" },
    Reject{ "hash_too_short",          "hash:3f9" },
    Reject{ "hash_not_hex",            "hash:xyz1" },
    Reject{ "hash_too_long",           "hash:" + std::string(65, 'a') },
    Reject{ "empty_value",             "name:" },
    Reject{ "unterminated_string",     "\"abc" },
    Reject{ "unbalanced_open",         "(((" },
    Reject{ "unbalanced_close",        "a)" },
    Reject{ "dangling_or",             "a OR" },
    Reject{ "dangling_not",            "-" },
    Reject{ "too_long",                std::string(5000, 'a') },
    Reject{ "too_deep_parens",         std::string(100, '(') + "a" + std::string(100, ')') },
    Reject{ "too_deep_not",            std::string(1000, '!') + "a" }
), CaseName);

// --- error positions (code point offsets, end exclusive) ---------------------

struct ErrorAt
{
    std::string name;
    std::string query;
    size_t      start;
    size_t      end;
};

void PrintTo(const ErrorAt& e, std::ostream* os) { *os << '"' << e.query << '"'; }

class PqlErrorAt : public ::testing::TestWithParam<ErrorAt> {};

TEST_P(PqlErrorAt, ReportsSpan)
{
    const auto& c = GetParam();

    try
    {
        PQL::Parse(c.query);
        FAIL() << "expected QueryError for: " << c.query;
    }
    catch (const QueryError& e)
    {
        EXPECT_EQ(e.start(), c.start) << c.query;
        EXPECT_EQ(e.end(),   c.end)   << c.query;
    }
}

INSTANTIATE_TEST_SUITE_P(Pql, PqlErrorAt, ::testing::Values(
    ErrorAt{ "unknown_field",      "ubuntu szie:>1gb",   7, 12 },
    ErrorAt{ "unknown_unit",       "size:>1zb",          6,  9 },
    ErrorAt{ "invalid_operator",   "name:>abc",          5,  6 },
    ErrorAt{ "utf8_offsets",       "åäö szie:1",         4,  9 }
), CaseName);