#include <gtest/gtest.h>

#include <ctime>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <libtorrent/load_torrent.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../../src/fields.hpp"
#include "../../src/query/pql.hpp"
#include "../../src/torrentclientdata.hpp"
#include "../../src/utils/hex.hpp"

using porla::Fields;
using porla::Query::PQL;
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

        [[nodiscard]] porla::Fields::Context Context() const
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
        { "name_contains",               "name:desktop",                          true  },
        { "name_contains_case",          "name:DESKTOP",                          true  },
        { "name_miss",                   "name:debian",                           false },
        { "name_eq",                     "name:=UBUNTU-24.04-desktop-amd64.iso",  true  },
        { "name_eq_partial",             "name:=ubuntu",                          false },
        { "name_glob",                   "name:ubuntu*.iso",                      true  },
        { "name_glob_anchored",          "name:*desktop",                         false },
        { "name_list",                   "name:debian,desktop",                   true  },
        { "name_quoted_no_split",        "name:\"debian,desktop\"",               false },
        { "save_path",                   "save_path:/downloads/",                 true  },
        { "category_contains",           "$userdata.category:lin",                true  },
        { "category_eq",                 "$userdata.category:=linux",             true  },
        { "category_no_client_data",     "$userdata.category:linux",              false, [](Torrent& t) { t.with_client_data = false; } },
        { "category_unset",              "$userdata.category:linux",              false, [](Torrent& t) { t.client_data.category.reset(); } },
        { "tags_exact",                  "$userdata.tags:iso",                    true  },
        { "tags_case",                   "$userdata.tags:ISO",                    true  },
        { "tags_no_substring",           "$userdata.tags:is",                     false },
        { "tags_glob",                   "$userdata.tags:l*",                     true  },
        { "tags_any",                    "$userdata.tags:foo,lts",                true  },
        { "tags_all",                    "$userdata.tags:iso $userdata.tags:foo", false },
        { "tags_no_client_data",         "$userdata.tags:iso",                    false, [](Torrent& t) { t.with_client_data = false; } },
        { "current_tracker",             "current_tracker:example.org",           true  }
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
        { "v1_prefix",               "info_hash:3f9a",                true  },
        { "v2_prefix",               "info_hash:a1b2",                true  },
        { "v1_full",                 "info_hash:" + std::string(kV1), true  },
        { "v2_full",                 "info_hash:" + std::string(kV2), true  },
        { "uppercase",               "info_hash:3F9AAC15",            true  },
        { "miss",                    "info_hash:0000",                false },
        { "list",                    "info_hash:0000,3f9a",           true  },
        { "v2_absent",               "info_hash:a1b2",                false, [](Torrent& t) { t.status.info_hashes.v2.clear(); } },
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
        { "gt",             "total_wanted:>1gb",             true  },
        { "lt",             "total_wanted:<1gb",             false },
        { "float_gte",      "total_wanted:>=1.5gb",          true  },
        { "float_gt",       "total_wanted:>1.5gib",          false },
        { "gib",            "total_wanted:>1gib",            true  },
        { "uppercase_unit", "total_wanted:>1GB",             true  },
        { "bytes_eq",       "total_wanted:1610612736",       true  },
        { "b_suffix",       "total_wanted:>10b",             true  },
        { "range",          "total_wanted:1gb..2gb",         true  },
        { "range_miss",     "total_wanted:2gb..3gb",         false },
        { "downloaded",     "total_wanted_done:>=1gb",       true  },
        { "uploaded",       "all_time_upload:>1mb",          true  },
        { "remaining",      "total_wanted_remaining:512mib", true  },
        { "gb_is_decimal",  "total_wanted:>1.6gb",           true },
        { "gib_is_binary",  "total_wanted:>1.6gib",          false }
    })), CaseName);

INSTANTIATE_TEST_SUITE_P(Rate, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t)
    {
        t.status.download_payload_rate = 2 * MiB;
        t.status.upload_payload_rate   = 0;
    },
    {
        { "dl_gt",               "download_payload_rate:>500kb", true  },
        { "dl_per_second",       "download_payload_rate:>1mb/s", true  },
        { "dl_lt",               "download_payload_rate:<1mb",   false },
        { "ul_zero",             "upload_payload_rate:0",        true  },
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
        { "added_relative_lt",           "added_time:<-1h",            true  },
        { "added_relative_gt",           "added_time:>-1h",            false },
        { "added_relative_bare_seconds", "added_time:<-3600",          true  },
        { "added_relative_minutes",      "added_time:<-59m",           true  },
        { "added_relative_days",         "added_time:>-5d",            true  },
        { "added_relative_months",       "added_time:>-1mo",           true  },
        { "added_relative_years",        "added_time:>-1y",            true  },
        { "added_relative_range",        "added_time:-2h..-1h",        true  },
        { "added_relative_bare_within",  "added_time:-2h",             true  },   // no operator = ">=": within the last 2h
        { "added_relative_bare_outside", "added_time:-1h",             false },   // 3601s ago is not within the last hour
        { "added_relative_mixed_range",  "added_time:2026-01-01..-1h", true },
        { "added_relative_boundary",     "added_time:<-1h",            false, [](Torrent& t) { t.now = kNow - 1; } },
        { "active_duration",             "active_duration:>=1d",       true  },
        { "seeding_duration",            "seeding_duration:>1w",       true  },
        { "finished_duration",           "finished_duration:0",        true  },
        { "eta",                         "eta:<1h",                    true,  [](Torrent& t) { t.status.total_wanted = 101 * MiB; t.status.total_wanted_done = 1 * MiB; t.status.download_payload_rate = 1 * MiB; } },
        { "eta_unknown_never_match",     "eta:>0",                     false, [](Torrent& t) { t.status.total_wanted = 101 * MiB; t.status.download_payload_rate = 0; } },
        { "eta_unknown_negated",         "-eta:<1h",                   true,  [](Torrent& t) { t.status.total_wanted = 101 * MiB; t.status.download_payload_rate = 0; } },
        { "next_announce_soon",          "next_announce:<1m",          true,  [](Torrent& t) { t.status.next_announce = lt::seconds(30); } },
        { "next_announce_later",         "next_announce:<1m",          false, [](Torrent& t) { t.status.next_announce = lt::minutes(5); } },
        { "next_announce_paused",        "next_announce:<1m",          false },
        { "last_upload_recent",          "last_upload:<5m",            true,  [](Torrent& t) { t.status.last_upload = lt::clock_type::now() - lt::minutes(1); } },
        { "last_upload_stale",           "last_upload:>5m",            true,  [](Torrent& t) { t.status.last_upload = lt::clock_type::now() - lt::minutes(10); } },
        { "last_upload_never",           "last_upload:>5m",            false },
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
        { "ratio_gte",   "ratio:>=2",        true  },
        { "ratio_float", "ratio:<2.5",       true  },
        { "ratio_lt",    "ratio:<1",         false },
        { "seeds",       "num_seeds:>=10",   true  },
        { "peers",       "num_peers:0",      true  },
        { "queue",       "queue_position:3", true  },
    })), CaseName);

// --- dates (server local time; run under a non-UTC TZ) -----------------------

INSTANTIATE_TEST_SUITE_P(Date, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t)
    {
        t.status.added_time     = Local(2026, 1, 1, 12, 0);
        t.status.completed_time = 0;
    },
    {
        { "day",               "added_time:2026-01-01",             true  },
        { "other_day",         "added_time:2025-12-31",             false },
        { "gt_excludes_day",   "added_time:>2026-01-01",            false },
        { "gte_includes_day",  "added_time:>=2026-01-01",           true  },
        { "lt_next_day",       "added_time:<2026-01-02",            true  },
        { "lte_includes_day",  "added_time:<=2026-01-01",           true  },
        { "range_inclusive",   "added_time:2025-12-01..2026-01-01", true  },
        { "minute",            "added_time:2026-01-01T12:00",       true  },
        { "gt_minute",         "added_time:>2026-01-01T12:00",      false },
        { "day_end_boundary",  "added_time:2026-01-01",             true,  [](Torrent& t) { t.status.added_time = Local(2026, 1, 1, 23, 59, 59); } },
        { "next_day_boundary", "added_time:2026-01-01",             false, [](Torrent& t) { t.status.added_time = Local(2026, 1, 2, 0, 0, 0); } },
        { "never_completed",   "completed_time:>2000-01-01",        false },
        { "completed",         "completed_time:2026-01-01",         true,  [](Torrent& t) { t.status.completed_time = Local(2026, 1, 1, 18, 0); } },
    })), CaseName);

// --- has: ------------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(Has, PqlMatch, ::testing::Values(
    Case{ "has_category",           "has:$userdata.category",                true,  [](Torrent& t) { t.client_data.category = "tv"; } },
    Case{ "has_category_unset",     "has:$userdata.category",                false },
    Case{ "has_category_no_data",   "has:$userdata.category",                false, [](Torrent& t) { t.with_client_data = false; } },
    Case{ "has_tags",               "has:$userdata.tags",                    true,  [](Torrent& t) { t.client_data.tags = { "a" }; } },
    Case{ "not_has_tags",           "-has:$userdata.tags",                   true  },
    Case{ "has_category_empty",     "has:$userdata.category",                false, [](Torrent& t) { t.client_data.category = ""; } },
    Case{ "has_any_of",             "has:$userdata.tags,$userdata.category", true,  [](Torrent& t) { t.client_data.category = "tv"; } },
    Case{ "has_completed",          "has:completed_time",                    true,  [](Torrent& t) { t.status.completed_time = kNow; } },
    Case{ "has_never_completed",    "has:completed_time",                    false },
    Case{ "has_eta_unknown",        "has:eta",                               false },
    Case{ "has_next_announce",      "has:next_announce",                     true,  [](Torrent& t) { t.status.next_announce = lt::seconds(30); } },
    Case{ "has_next_announce_none", "has:next_announce",                     false },
    Case{ "has_last_upload",        "has:last_upload",                       true,  [](Torrent& t) { t.status.last_upload = lt::clock_type::now() - lt::minutes(1); } },
    Case{ "has_last_upload_none",   "has:last_upload",                       false }
), CaseName);

INSTANTIATE_TEST_SUITE_P(Flags, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t) { t.status.flags = lt::torrent_flags::paused | lt::torrent_flags::auto_managed; },
    {
        { "flags_single",       "flags:paused",                          true  },
        { "flags_case",         "flags:PAUSED",                          true  },
        { "flags_all_of",       "flags:paused,auto_managed",             true  },
        { "flags_all_of_miss",  "flags:paused,sequential_download",      false },
        { "flags_not_set",      "flags:~sequential_download",            true  },
        { "flags_negated_set",  "flags:auto_managed,~paused",            false },
        { "flags_running",      "flags:auto_managed,~paused",            true,  [](Torrent& t) { t.status.flags = lt::torrent_flags::auto_managed; } },
        { "flags_term_not",     "-flags:paused",                         false },
        { "flags_any_via_or",   "flags:seed_mode OR flags:paused",       true  },
        { "flags_none_set",     "flags:paused",                          false, [](Torrent& t) { t.status.flags = {}; } },
    })), CaseName);

TEST(PqlRelative, ResolvesAgainstEvaluationTime)
{
    const auto filter = PQL::Parse("added_time:>-1h");

    Torrent t;
    t.status.added_time = kNow - 1800;

    EXPECT_TRUE(filter(t.Context()));

    t.now = kNow + 3600;
    EXPECT_FALSE(filter(t.Context()));
}

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
    Reject{ "unknown_field",           "totl:1",                    "did you mean 'total'" },
    Reject{ "unknown_has",             "has:bogus" },
    Reject{ "unknown_size_unit",       "total:>1zb",                 "zb" },
    Reject{ "bits_unit_dropped",       "download_payload_rate:>1mbps",                 "mbps" },
    Reject{ "duration_unit_on_total",  "total:>1h" },
    Reject{ "size_unit_on_duration",   "eta:>1gb" },
    Reject{ "op_on_text",              "name:>abc" },
    Reject{ "op_on_tag",               "$userdata.tags:>a" },
    Reject{ "text_for_total",          "total:abc" },
    Reject{ "list_on_total",           "total:1gb,2gb" },
    Reject{ "glob_on_total",           "total:1*" },
    Reject{ "range_reversed",          "total:2gb..1gb" },
    Reject{ "progress_over_100",       "progress:>101" },
    Reject{ "negative_number",         "progress:>-1" },
    Reject{ "bad_month",               "added_time:2026-13-01" },
    Reject{ "bad_day",                 "added_time:2026-02-30" },
    Reject{ "bad_date_format",         "added_time:01/01/2026" },
    Reject{ "hash_too_short",          "info_hash:3f9" },
    Reject{ "hash_not_hex",            "info_hash:xyz1" },
    Reject{ "hash_too_long",           "info_hash:" + std::string(65, 'a') },
    Reject{ "empty_value",             "name:" },
    Reject{ "unterminated_string",     "\"abc" },
    Reject{ "unbalanced_open",         "(((" },
    Reject{ "unbalanced_close",        "a)" },
    Reject{ "dangling_or",             "a OR" },
    Reject{ "dangling_not",            "-" },
    Reject{ "too_long",                std::string(5000, 'a') },
    Reject{ "too_deep_parens",         std::string(100, '(') + "a" + std::string(100, ')') },
    Reject{ "too_deep_not",            std::string(1000, '!') + "a" },
    Reject{ "relative_needs_minus",    "added_time:>1h",         "did you mean '-1h'" },
    Reject{ "relative_eq",             "added_time:=-1h",        "relative" },
    Reject{ "relative_range_reversed", "added_time:-1h..-2h",    "reversed" },
    Reject{ "relative_bad_unit",       "added_time:>-1gb",       "gb" },
    Reject{ "flags_unknown",           "flags:bogus",            "Unknown flag" },
    Reject{ "flags_typo",              "flags:pasued",           "did you mean 'paused'" },
    Reject{ "flags_contradiction",     "flags:paused,~paused",   "more than once" },
    Reject{ "flags_operator",          "flags:>paused",          "not valid" },
    Reject{ "flags_eq",                "flags:=paused",          "not valid" },
    Reject{ "flags_lone_tilde",        "flags:~",                "Unknown flag" },
    Reject{ "relative_hint_unit",       "added_time:>1h",        "did you mean '-1h'" },
    Reject{ "relative_hint_fraction",   "added_time:1.5d",       "did you mean '-1.5d'" },
    Reject{ "no_hint_compact_date",     "added_time:20260101",   "Expected a date (YYYY-MM-DD" },
    Reject{ "no_hint_slash_date",       "added_time:01/01/2026", "Expected a date (YYYY-MM-DD" },
    Reject{ "no_hint_size_unit",        "added_time:1gb",        "Expected a date (YYYY-MM-DD" }
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
    ErrorAt{ "unknown_field",    "ubuntu totl:>1gb", 7, 12 },
    ErrorAt{ "unknown_unit",     "total:>1zb",       7, 10 },
    ErrorAt{ "invalid_operator", "name:>abc",        5, 6 },
    ErrorAt{ "utf8_offsets",     "åäö totl:1",       4, 9 },
    ErrorAt{ "bool_bad_value",  "is_seeding:maybe", 11, 16 },
    ErrorAt{ "bool_operator",   "is_seeding:>true", 11, 12 }
), CaseName);

INSTANTIATE_TEST_SUITE_P(State, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t) { t.status.state = lt::torrent_status::seeding; },
    {
        { "state_match",            "state:seeding",                                 true  },
        { "state_miss",             "state:downloading",                             false },
        { "state_case",             "state:SEEDING",                                 true  },
        { "state_eq",               "state:=seeding",                                true  },
        { "state_quoted",           "state:\"seeding\"",                             true  },
        { "state_any_of",           "state:downloading,seeding",                     true  },
        { "state_negated",          "-state:seeding",                                false },
        { "state_checking_files",   "state:checking_files,checking_resume_data",     true,  [](Torrent& t) { t.status.state = lt::torrent_status::checking_files; } },
        { "state_checking_resume",  "state:checking_files,checking_resume_data",     true,  [](Torrent& t) { t.status.state = lt::torrent_status::checking_resume_data; } },
        { "state_metadata",         "state:downloading_metadata",                    true,  [](Torrent& t) { t.status.state = lt::torrent_status::downloading_metadata; } },
        { "state_finished",         "state:finished",                                true,  [](Torrent& t) { t.status.state = lt::torrent_status::finished; } },
    })), CaseName);

// --- bool --------------------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(Bool, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t)
    {
        t.status.is_seeding     = true;
        t.status.has_metadata   = true;
        t.status.moving_storage = false;
    },
    {
        { "bool_true",              "is_seeding:true",        true  },
        { "bool_false_miss",        "is_seeding:false",       false },
        { "bool_false",             "moving_storage:false",   true  },
        { "bool_eq",                "is_seeding:=true",       true  },
        { "bool_yes",               "has_metadata:yes",       true  },
        { "bool_no",                "moving_storage:no",      true  },
        { "bool_one",               "is_seeding:1",           true  },
        { "bool_zero",              "moving_storage:0",       true  },
        { "bool_case",              "is_seeding:TRUE",        true  },
        { "bool_quoted",            "is_seeding:\"true\"",    true  },
        { "bool_negated",           "-is_seeding:true",       false },
        { "errc_unset",             "errc:false",             true  },
        { "errc_set",               "errc:true",              true,  [](Torrent& t) { t.status.errc = lt::errors::make_error_code(lt::errors::file_too_short); } },
    })), CaseName);

// --- rate limits -------------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(Limits, PqlMatch, ::testing::Values(
    // -1 means unlimited and must never match a range
    Case{ "upload_limit_unlimited",       "upload_limit:<1mb/s",       false },
    Case{ "download_limit_unlimited",     "download_limit:<1mb/s",     false },
    Case{ "upload_limit_capped",          "upload_limit:<1mb/s",       true,  [](Torrent& t) { t.status.upload_limit = 500000; } },
    Case{ "upload_limit_above",           "upload_limit:<1mb/s",       false, [](Torrent& t) { t.status.upload_limit = 2000000; } },
    Case{ "download_limit_capped",        "download_limit:500kb..1mb", true,  [](Torrent& t) { t.status.download_limit = 750000; } },
    Case{ "has_upload_limit",             "has:upload_limit",          true,  [](Torrent& t) { t.status.upload_limit = 1; } },
    Case{ "has_upload_limit_unlimited",   "has:upload_limit",          false },
    Case{ "unlimited_via_not_has",        "-has:download_limit",       true  }
), CaseName);

// --- values that mean "not set" ----------------------------------------------

INSTANTIATE_TEST_SUITE_P(Unset, PqlMatch, ::testing::Values(
    Case{ "queue_position_unqueued",      "queue_position:<5",               false, [](Torrent& t) { t.status.queue_position = lt::queue_position_t{ -1 }; } },
    Case{ "has_queue_position_unqueued",  "has:queue_position",              false, [](Torrent& t) { t.status.queue_position = lt::queue_position_t{ -1 }; } },
    Case{ "has_queue_position",           "has:queue_position",              true,  [](Torrent& t) { t.status.queue_position = lt::queue_position_t{ 0 }; } },
    Case{ "last_seen_complete_unset",     "last_seen_complete:<2020-01-01",  false },
    Case{ "has_last_seen_complete_unset", "has:last_seen_complete",          false },
    Case{ "has_last_seen_complete",       "has:last_seen_complete",          true,  [](Torrent& t) { t.status.last_seen_complete = kNow; } }
), CaseName);

// --- last_upload / last_download ---------------------------------------------

INSTANTIATE_TEST_SUITE_P(LastActivity, PqlMatch, ::testing::Values(
    // libtorrent restores pre-boot activity from resume data as a negative steady-clock time point;
    // only exactly the epoch means "never"
    Case{ "last_upload_before_boot",      "last_upload:>5m",    true,  [](Torrent& t) { t.status.last_upload = lt::time_point(-lt::hours(24 * 365)); } },
    Case{ "has_last_upload_before_boot",  "has:last_upload",    true,  [](Torrent& t) { t.status.last_upload = lt::time_point(-lt::hours(24 * 365)); } },
    Case{ "last_upload_epoch_is_unset",   "has:last_upload",    false, [](Torrent& t) { t.status.last_upload = lt::time_point{}; } },
    Case{ "last_download_recent",         "last_download:<5m",  true,  [](Torrent& t) { t.status.last_download = lt::clock_type::now() - lt::minutes(1); } },
    Case{ "last_download_stale",          "last_download:>5m",  true,  [](Torrent& t) { t.status.last_download = lt::clock_type::now() - lt::minutes(10); } },
    Case{ "last_download_never",          "last_download:>5m",  false },
    Case{ "last_download_never_or_stale", "last_download:>5m OR -has:last_download", true },
    Case{ "has_last_download",            "has:last_download",  true,  [](Torrent& t) { t.status.last_download = lt::clock_type::now() - lt::minutes(1); } }
), CaseName);

// --- ratio_real --------------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(RatioReal, PqlMatch, ::testing::ValuesIn(With(
    [](Torrent& t)
    {
        t.status.all_time_upload   = 2 * GiB;
        t.status.all_time_download = 1 * GiB;
    },
    {
        { "ratio_real",               "ratio_real:2",       true  },
        { "ratio_real_never_dl",      "ratio_real:>=9999",  true,  [](Torrent& t) { t.status.all_time_download = 0; } },
        { "ratio_real_nothing",       "ratio_real:0",       true,  [](Torrent& t) { t.status.all_time_upload = 0; t.status.all_time_download = 0; } },
    })), CaseName);

// --- more rejects ------------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(Fields, PqlRejects, ::testing::Values(
    Reject{ "state_unknown",          "state:bogus",                         "Unknown torrent state 'bogus'" },
    Reject{ "state_glob",             "state:seed*",                         "Unknown torrent state" },
    Reject{ "state_operator",         "state:>seeding",                      "not valid" },
    Reject{ "state_quoted_list",      "state:\"seeding,downloading\"",       "Unknown torrent state" },
    Reject{ "bool_bad_value",         "is_seeding:maybe",                    "Unexpected value for boolean" },
    Reject{ "bool_list",              "is_seeding:true,false",               "Unexpected value for boolean" },
    Reject{ "bool_operator",          "is_seeding:>true",                    "not valid" },
    Reject{ "has_bool",               "has:moving_storage",                  "use 'moving_storage:true'" },
    Reject{ "has_state",              "has:state",                           "always has a value" },
    Reject{ "has_flags",              "has:flags",                           "always has a value" },
    Reject{ "has_hash",               "has:info_hash",                       "always has a value" },
    Reject{ "has_operator",           "has:>completed_time",                 "does not take an operator" },
    Reject{ "has_unknown_in_list",    "has:$userdata.tags,bogus",            "Unknown field 'bogus'" },
    Reject{ "has_typo_hint",          "has:$userdata.tag",                   "did you mean '$userdata.tags'" },
    Reject{ "number_too_large",       "total:1" + std::string(400, '0'),     "out of range" },
    Reject{ "number_underflow",       "total:0." + std::string(400, '0') + "1", "out of range" },
    Reject{ "relative_too_large",     "added_time:>-1" + std::string(400, '0') + "h", "out of range" },
    Reject{ "limit_number_unit",      "upload_limit:<1h",                    "Unknown rate unit 'h'" },
    Reject{ "userdata_typo_hint",     "$userdata.categroy:x",                "did you mean '$userdata.category'" }
), CaseName);

INSTANTIATE_TEST_SUITE_P(Fields, PqlErrorAt, ::testing::Values(
    ErrorAt{ "state_unknown",   "state:bogus",       6, 11 },
    ErrorAt{ "has_unknown",     "has:bogus",         4, 9  },
    ErrorAt{ "flags_unknown",   "flags:bogus",       6, 11 },
    ErrorAt{ "number_too_large", "total:1" + std::string(400, '0'), 6, 407 }
), CaseName);
