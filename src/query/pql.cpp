#include "pql.hpp"

#include <antlr4-runtime.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <ctime>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "_aux/PqlLexer.h"
#include "_aux/PqlParser.h"

#include "../torrentclientdata.hpp"
#include "../utils/eta.hpp"
#include "../utils/hex.hpp"
#include "../utils/ratio.hpp"

using porla::Query::Filter;
using porla::Query::PQL;
using porla::Query::QueryContext;
using porla::Query::QueryError;

using P = PqlParser;

namespace
{
    constexpr std::size_t kMaxQueryLength = 4096;
    constexpr int         kMaxQueryDepth  = 64;

    struct Span
    {
        size_t start;
        size_t end;   // exclusive
    };

    Span SpanOf(const antlr4::Token* token)
    {
        const size_t start = token->getStartIndex();
        const size_t stop  = token->getStopIndex();

        // EOF has stop == start - 1
        return { start, std::max(start, stop + 1) };
    }

    Span SpanOf(antlr4::tree::TerminalNode* node) { return SpanOf(node->getSymbol()); }

    Span SpanOf(const antlr4::ParserRuleContext* ctx)
    {
        return { SpanOf(ctx->getStart()).start, SpanOf(ctx->getStop()).end };
    }

    [[noreturn]] void Fail(const std::string& message, Span span)
    {
        throw QueryError(message, span.start, span.end);
    }

    class ThrowingErrorListener : public antlr4::BaseErrorListener
    {
    public:
        void syntaxError(
            antlr4::Recognizer* recognizer,
            antlr4::Token*      offending,
            size_t              /* line */,
            size_t              /* char_pos */,
            const std::string&  msg,
            std::exception_ptr  /* e */) override
        {
            if (offending != nullptr)
            {
                Fail(msg, SpanOf(offending));
            }

            // lexer errors have no token, only the current input position
            auto* lexer = dynamic_cast<antlr4::Lexer*>(recognizer);
            const size_t pos  = lexer != nullptr ? lexer->getCharIndex() : 0;

            Fail(msg, { pos, pos + 1 });
        }
    };

    void CheckDepth(antlr4::CommonTokenStream& tokens)
    {
        int depth   = 0;
        int not_run = 0;

        for (const auto* token : tokens.getTokens())
        {
            switch (token->getType())
            {
            case P::LPAREN: depth++;   not_run = 0; break;
            case P::RPAREN: depth--;   not_run = 0; break;
            case P::NOT:    not_run++;              break;
            default:        not_run = 0;            break;
            }

            if (depth > kMaxQueryDepth || not_run > kMaxQueryDepth)
            {
                Fail("Query nested too deeply (max " + std::to_string(kMaxQueryDepth) + ")", SpanOf(token));
            }
        }
    }

    Filter All(std::vector<Filter> filters)
    {
        if (filters.size() == 1) { return std::move(filters.front()); }

        return [filters = std::move(filters)](const QueryContext& ctx)
        {
            return std::all_of(filters.begin(), filters.end(), [&](const auto& f) { return f(ctx); });
        };
    }

    Filter Any(std::vector<Filter> filters)
    {
        if (filters.size() == 1) { return std::move(filters.front()); }

        return [filters = std::move(filters)](const QueryContext& ctx)
        {
            return std::any_of(filters.begin(), filters.end(), [&](const auto& f) { return f(ctx); });
        };
    }

    Filter Not(Filter inner)
    {
        return [inner = std::move(inner)](const QueryContext& ctx) { return !inner(ctx); };
    }

    enum class Oper { None, Eq, Gt, Gte, Lt, Lte };

    struct Value
    {
        std::string text;     // unquoted and unescaped
        bool        quoted;   // quoted = literal: no list, range or glob handling
        Span        span;
    };

    std::string Unescape(std::string_view quoted)
    {
        std::string out;
        out.reserve(quoted.size());

        for (size_t i = 1; i + 1 < quoted.size(); i++)
        {
            if (quoted[i] == '\\' && i + 2 < quoted.size()) { i++; }
            out += quoted[i];
        }

        return out;
    }

    Value ValueOf(antlr4::tree::TerminalNode* word, antlr4::tree::TerminalNode* string)
    {
        if (string != nullptr)
        {
            return { Unescape(string->getText()), true, SpanOf(string) };
        }

        return { word->getText(), false, SpanOf(word) };
    }

    Oper OperOf(P::OpContext* op)
    {
        if (op == nullptr)   { return Oper::None; }
        if (op->V_GTE())     { return Oper::Gte; }
        if (op->V_LTE())     { return Oper::Lte; }
        if (op->V_GT())      { return Oper::Gt; }
        if (op->V_LT())      { return Oper::Lt; }
        return Oper::Eq;
    }

    // --- text helpers ----------------------------------------------------------

    std::string ToLower(std::string_view s)
    {
        std::string out(s);
        std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return std::tolower(c); });
        return out;
    }

    bool IsHex(std::string_view s)
    {
        return std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isxdigit(c); });
    }

    bool LooksLikeHash(std::string_view s)
    {
        return s.size() >= 8 && s.size() <= 64 && IsHex(s);
    }

    char LowerAscii(char c)
    {
        return c >= 'A' && c <= 'Z' ? static_cast<char>(c + ('a' - 'A')) : c;
    }

    bool EqualsIgnoreCase(char text_char, char lower_char) { return LowerAscii(text_char) == lower_char; }

    // Pattern is lowercase; text is compared ignoring ASCII case.
    bool GlobMatch(std::string_view pattern, std::string_view text)
    {
        size_t p = 0, t = 0, star = std::string_view::npos, mark = 0;

        while (t < text.size())
        {
            if (p < pattern.size() && pattern[p] == '*')
            {
                star = p++;
                mark = t;
            }
            else if (p < pattern.size() && pattern[p] == LowerAscii(text[t]))
            {
                p++;
                t++;
            }
            else if (star != std::string_view::npos)
            {
                p = star + 1;
                t = ++mark;
            }
            else
            {
                return false;
            }
        }

        while (p < pattern.size() && pattern[p] == '*') { p++; }

        return p == pattern.size();
    }

    std::string OperText(Oper oper)
    {
        switch (oper)
        {
        case Oper::Eq:  return "=";
        case Oper::Gt:  return ">";
        case Oper::Gte: return ">=";
        case Oper::Lt:  return "<";
        case Oper::Lte: return "<=";
        default:        return "";
        }
    }

    // Quoted values are literal. Unquoted values split on ','.
    std::vector<std::string> ListOf(const Value& value)
    {
        if (value.quoted) { return { value.text }; }

        std::vector<std::string> items;
        size_t start = 0;

        while (true)
        {
            const size_t comma = value.text.find(',', start);
            auto item = value.text.substr(start, comma == std::string::npos ? std::string::npos : comma - start);

            if (item.empty())
            {
                Fail("Empty item in list", value.span);
            }

            items.push_back(std::move(item));

            if (comma == std::string::npos) { break; }
            start = comma + 1;
        }

        return items;
    }

    struct TextMatcher
    {
        enum class Mode { Contains, Equals, Glob };

        Mode        mode;
        std::string needle;   // lowercase

        // text in any case, needle already lowercase
        [[nodiscard]] bool Matches(std::string_view text) const
        {
            switch (mode)
            {
            case Mode::Contains: return std::search(text.begin(), text.end(), needle.begin(), needle.end(), EqualsIgnoreCase) != text.end();
            case Mode::Equals:   return std::equal(text.begin(), text.end(), needle.begin(), needle.end(), EqualsIgnoreCase);
            case Mode::Glob:     return GlobMatch(needle, text);
            }

            return false;
        }
    };

    std::vector<TextMatcher> MatchersOf(const Value& value, TextMatcher::Mode default_mode)
    {
        std::vector<TextMatcher> matchers;

        for (auto& item : ListOf(value))
        {
            const auto mode = !value.quoted && item.find('*') != std::string::npos
                ? TextMatcher::Mode::Glob
                : default_mode;

            matchers.push_back({ mode, ToLower(item) });
        }

        return matchers;
    }

    // --- registry --------------------------------------------------------------

    enum class Kind { Text, Tag, Hash, Size, Rate, Duration, Number, Percent, Date };

    using NumberGetter = std::optional<double> (*)(const QueryContext&);
    using TextGetter   = std::optional<std::string_view> (*)(const QueryContext&);

    struct FieldDef
    {
        std::string_view name;
        Kind             kind;
        NumberGetter     number = nullptr;
        TextGetter       text   = nullptr;
    };

    std::optional<double> Seconds(std::chrono::seconds s) { return static_cast<double>(s.count()); }

    std::optional<double> Timestamp(std::time_t t)
    {
        if (t <= 0) { return std::nullopt; }
        return static_cast<double>(t);
    }

    const std::vector<FieldDef>& Fields()
    {
        static const std::vector<FieldDef> fields =
        {
            { "name",          Kind::Text,     nullptr, [](const QueryContext& c) -> std::optional<std::string_view> { return c.status.name; } },
            { "path",          Kind::Text,     nullptr, [](const QueryContext& c) -> std::optional<std::string_view> { return c.status.save_path; } },
            { "tracker",       Kind::Text,     nullptr, [](const QueryContext& c) -> std::optional<std::string_view> { return c.status.current_tracker; } },
            { "category",      Kind::Text,     nullptr, [](const QueryContext& c) -> std::optional<std::string_view>
                {
                    if (c.client_data == nullptr || !c.client_data->category.has_value()) { return std::nullopt; }
                    return *c.client_data->category;
                } },
            { "tag",           Kind::Tag },
            { "hash",          Kind::Hash },

            { "size",          Kind::Size,     [](const QueryContext& c) -> std::optional<double> { return c.status.total_wanted; } },
            { "downloaded",    Kind::Size,     [](const QueryContext& c) -> std::optional<double> { return c.status.total_done; } },
            { "uploaded",      Kind::Size,     [](const QueryContext& c) -> std::optional<double> { return c.status.all_time_upload; } },
            { "remaining",     Kind::Size,     [](const QueryContext& c) -> std::optional<double> { return c.status.total_wanted - c.status.total_wanted_done; } },

            { "dl",            Kind::Rate,     [](const QueryContext& c) -> std::optional<double> { return c.status.download_payload_rate; } },
            { "download_rate", Kind::Rate,     [](const QueryContext& c) -> std::optional<double> { return c.status.download_payload_rate; } },
            { "ul",            Kind::Rate,     [](const QueryContext& c) -> std::optional<double> { return c.status.upload_payload_rate; } },
            { "upload_rate",   Kind::Rate,     [](const QueryContext& c) -> std::optional<double> { return c.status.upload_payload_rate; } },

            { "age",           Kind::Duration, [](const QueryContext& c) -> std::optional<double> { return static_cast<double>(c.now - c.status.added_time); } },
            { "eta",           Kind::Duration, [](const QueryContext& c) -> std::optional<double>
                {
                    const auto eta = porla::Utils::ETA(c.status);
                    if (eta.count() < 0) { return std::nullopt; }
                    return static_cast<double>(eta.count());
                } },
            { "active_time",   Kind::Duration, [](const QueryContext& c) { return Seconds(c.status.active_duration); } },
            { "seed_time",     Kind::Duration, [](const QueryContext& c) { return Seconds(c.status.seeding_duration); } },
            { "finished_time", Kind::Duration, [](const QueryContext& c) { return Seconds(c.status.finished_duration); } },

            { "progress",      Kind::Percent,  [](const QueryContext& c) -> std::optional<double> { return c.status.progress_ppm; } },

            { "ratio",         Kind::Number,   [](const QueryContext& c) -> std::optional<double> { return porla::Utils::Ratio(c.status); } },
            { "seeds",         Kind::Number,   [](const QueryContext& c) -> std::optional<double> { return c.status.num_seeds; } },
            { "peers",         Kind::Number,   [](const QueryContext& c) -> std::optional<double> { return c.status.num_peers; } },
            { "queue",         Kind::Number,   [](const QueryContext& c) -> std::optional<double> { return static_cast<int>(c.status.queue_position); } },

            { "added",         Kind::Date,     [](const QueryContext& c) { return Timestamp(c.status.added_time); } },
            { "completed",     Kind::Date,     [](const QueryContext& c) { return Timestamp(c.status.completed_time); } },
        };

        return fields;
    }

    const FieldDef* FindField(std::string_view name)
    {
        for (const auto& field : Fields())
        {
            if (field.name == name) { return &field; }
        }

        return nullptr;
    }

    size_t Levenshtein(std::string_view a, std::string_view b)
    {
        std::vector<size_t> row(b.size() + 1);
        for (size_t j = 0; j <= b.size(); j++) { row[j] = j; }

        for (size_t i = 1; i <= a.size(); i++)
        {
            size_t diag = row[0];
            row[0] = i;

            for (size_t j = 1; j <= b.size(); j++)
            {
                const size_t up = row[j];
                row[j] = std::min({ row[j] + 1, row[j - 1] + 1, diag + (a[i - 1] == b[j - 1] ? 0 : 1) });
                diag = up;
            }
        }

        return row[b.size()];
    }

    std::string Suggest(std::string_view name)
    {
        static const std::map<std::string_view, std::string_view> renamed =
        {
            { "active_duration",   "active_time" },
            { "finished_duration", "finished_time" },
            { "save_path",         "path" },
            { "seeding_duration",  "seed_time" },
            { "tags",              "tag" },
        };

        if (const auto it = renamed.find(name); it != renamed.end())
        {
            return " - did you mean '" + std::string(it->second) + "'?";
        }

        std::string_view best;
        size_t best_distance = 3;

        for (const auto& field : Fields())
        {
            if (const auto d = Levenshtein(name, field.name); d < best_distance)
            {
                best          = field.name;
                best_distance = d;
            }
        }

        return best.empty() ? "" : " - did you mean '" + std::string(best) + "'?";
    }

    // --- is: / has: ------------------------------------------------------------

    using Predicate = bool (*)(const QueryContext&);

    Filter BuildFlag(std::string_view field, const Value& value)
    {
        static const std::map<std::string_view, Predicate> is_flags =
        {
            { "active",      [](const QueryContext& c) { return c.status.download_payload_rate > 0 || c.status.upload_payload_rate > 0; } },
            { "checking",    [](const QueryContext& c) { return c.status.state == lt::torrent_status::checking_files || c.status.state == lt::torrent_status::checking_resume_data; } },
            { "downloading", [](const QueryContext& c) { return c.status.state == lt::torrent_status::downloading; } },
            { "error",       [](const QueryContext& c) { return static_cast<bool>(c.status.errc); } },
            { "finished",    [](const QueryContext& c) { return c.status.state == lt::torrent_status::finished; } },
            { "moving",      [](const QueryContext& c) { return c.status.moving_storage; } },
            { "paused",      [](const QueryContext& c) { return (c.status.flags & lt::torrent_flags::paused) == lt::torrent_flags::paused; } },
            { "private",     [](const QueryContext& c) { const auto ti = c.status.torrent_file.lock(); return ti != nullptr && ti->priv(); } },
            { "queued",      [](const QueryContext& c)
                {
                    const auto queued = lt::torrent_flags::paused | lt::torrent_flags::auto_managed;
                    return (c.status.flags & queued) == queued;
                } },
            { "seeding",     [](const QueryContext& c) { return c.status.state == lt::torrent_status::seeding; } },
            { "stalled",     [](const QueryContext& c) { return c.status.state == lt::torrent_status::downloading && c.status.download_payload_rate == 0; } },
        };

        static const std::map<std::string_view, Predicate> has_flags =
        {
            { "category",    [](const QueryContext& c) { return c.client_data != nullptr && c.client_data->category.has_value(); } },
            { "error",       [](const QueryContext& c) { return static_cast<bool>(c.status.errc); } },
            { "metadata",    [](const QueryContext& c) { return c.status.has_metadata; } },
            { "tags",        [](const QueryContext& c) { return c.client_data != nullptr && !c.client_data->tags.empty(); } },
        };

        const auto& flags = field == "is" ? is_flags : has_flags;

        std::vector<Predicate> predicates;

        for (const auto& item : ListOf(value))
        {
            const auto it = flags.find(ToLower(item));

            if (it == flags.end())
            {
                Fail("Unknown flag '" + item + "' for '" + std::string(field) + ":'", value.span);
            }

            predicates.push_back(it->second);
        }

        return [predicates](const QueryContext& ctx)
        {
            return std::any_of(predicates.begin(), predicates.end(), [&](auto p) { return p(ctx); });
        };
    }

    // --- text, tag, hash -------------------------------------------------------

    void RequireEqOrNone(Oper oper, Span oper_span, std::string_view what)
    {
        if (oper != Oper::None && oper != Oper::Eq)
        {
            Fail("'" + OperText(oper) + "' is not valid for " + std::string(what), oper_span);
        }
    }

    Filter BuildText(const FieldDef& def, Oper oper, Span oper_span, const Value& value)
    {
        RequireEqOrNone(oper, oper_span, "text field '" + std::string(def.name) + "'");

        auto matchers = MatchersOf(value, oper == Oper::Eq ? TextMatcher::Mode::Equals : TextMatcher::Mode::Contains);

        return [get = def.text, matchers = std::move(matchers)](const QueryContext& ctx)
        {
            const auto text = get(ctx);
            if (!text.has_value()) { return false; }

            return std::any_of(matchers.begin(), matchers.end(), [&](const auto& m) { return m.Matches(*text); });
        };
    }

    Filter BuildTag(Oper oper, Span oper_span, const Value& value)
    {
        RequireEqOrNone(oper, oper_span, "field 'tag'");

        auto matchers = MatchersOf(value, TextMatcher::Mode::Equals);

        return [matchers = std::move(matchers)](const QueryContext& ctx)
        {
            if (ctx.client_data == nullptr) { return false; }

            return std::any_of(ctx.client_data->tags.begin(), ctx.client_data->tags.end(), [&](const auto& tag)
            {
                return std::any_of(matchers.begin(), matchers.end(), [&](const auto& m) { return m.Matches(tag); });
            });
        };
    }

    bool HashHasPrefix(const lt::info_hash_t& ih, const std::vector<std::string>& prefixes)
    {
        const auto check = [&](const auto& h)
        {
            const auto* bytes = reinterpret_cast<const unsigned char*>(h.data());
            const auto  size  = static_cast<size_t>(h.size());

            return std::any_of(prefixes.begin(), prefixes.end(), [&](const std::string& p)
            {
                if (p.size() > size * 2) { return false; }

                for (size_t i = 0; i < p.size(); i++)
                {
                    const unsigned nibble = i % 2 == 0 ? bytes[i / 2] >> 4 : bytes[i / 2] & 0x0f;
                    if ("0123456789abcdef"[nibble] != p[i]) { return false; }
                }

                return true;
            });
        };

        return (ih.has_v1() && check(ih.v1))
            || (ih.has_v2() && check(ih.v2));
    }

    Filter BuildHash(Oper oper, Span oper_span, const Value& value)
    {
        RequireEqOrNone(oper, oper_span, "field 'hash'");

        std::vector<std::string> prefixes;

        for (const auto& item : ListOf(value))
        {
            if (item.size() < 4 || item.size() > 64 || !IsHex(item))
            {
                Fail("Expected 4-64 hex characters for 'hash:'", value.span);
            }

            prefixes.push_back(ToLower(item));
        }

        return [prefixes = std::move(prefixes)](const QueryContext& ctx)
        {
            return HashHasPrefix(ctx.status.info_hashes, prefixes);
        };
    }

    Filter BuildFreeText(const Value& value)
    {
        const auto lower   = ToLower(value.text);
        const bool as_hash = !value.quoted && LooksLikeHash(lower);

        return [lower, as_hash, prefixes = std::vector<std::string>{ lower }](const QueryContext& ctx)
        {
            const std::string_view name = ctx.status.name;
            if (std::search(name.begin(), name.end(), lower.begin(), lower.end(), EqualsIgnoreCase) != name.end()) { return true; }
            return as_hash && HashHasPrefix(ctx.status.info_hashes, prefixes);
        };
    }

    // --- numeric ---------------------------------------------------------------

    // Closed interval. A plain number is [v, v]; a date is its whole day or minute.
    struct Interval
    {
        double lo;
        double hi;

        [[nodiscard]] bool Contains(double v) const { return v >= lo && v <= hi; }
    };

    constexpr double kInf = std::numeric_limits<double>::infinity();

    std::string_view KindName(Kind kind)
    {
        switch (kind)
        {
        case Kind::Size:     return "size";
        case Kind::Rate:     return "rate";
        case Kind::Duration: return "duration";
        case Kind::Percent:  return "percent";
        case Kind::Date:     return "date";
        default:             return "number";
        }
    }

    std::optional<double> UnitMultiplier(Kind kind, std::string_view unit)
    {
        static const std::map<std::string_view, double> sizes =
        {
            { "",   1 }, { "b",  1 },

            // decimal (SI) units
            { "kb", 1e3 },
            { "mb", 1e6 },
            { "gb", 1e9 },
            { "tb", 1e12 },
            { "pb", 1e15 },

            // binary (IEC) units
            { "kib", 1024. },
            { "mib", 1024. * 1024 },
            { "gib", 1024. * 1024 * 1024 },
            { "tib", 1024. * 1024 * 1024 * 1024 },
            { "pib", 1024. * 1024 * 1024 * 1024 * 1024 },
        };

        static const std::map<std::string_view, double> durations =
        {
            { "",  1 },
            { "s", 1 },
            { "m", 60 },
            { "h", 3600 },
            { "d", 86400 },
            { "w", 604800 },
            { "mo", 30 * 86400 },
            { "y", 365 * 86400 },
        };

        const auto find = [](const auto& map, std::string_view key) -> std::optional<double>
        {
            const auto it = map.find(key);
            if (it == map.end()) { return std::nullopt; }
            return it->second;
        };

        switch (kind)
        {
        case Kind::Size:     return find(sizes, unit);
        case Kind::Rate:     return find(sizes, unit.ends_with("/s") ? unit.substr(0, unit.size() - 2) : unit);
        case Kind::Duration: return find(durations, unit);
        case Kind::Percent:  return unit.empty() || unit == "%" ? std::optional(10000.) : std::nullopt;   // percent -> ppm
        default:             return unit.empty() ? std::optional(1.) : std::nullopt;
        }
    }

    std::optional<Interval> ParseDate(std::string_view text)
    {
        const auto digits = [&](size_t pos, size_t len) -> std::optional<int>
        {
            int out = 0;
            const auto* begin = text.data() + pos;
            const auto  res   = std::from_chars(begin, begin + len, out);
            if (res.ec != std::errc() || res.ptr != begin + len) { return std::nullopt; }
            return out;
        };

        const bool has_time = text.size() == 16;

        if ((text.size() != 10 && !has_time)
            || text[4] != '-' || text[7] != '-'
            || (has_time && (text[10] != 'T' || text[13] != ':')))
        {
            return std::nullopt;
        }

        const auto year  = digits(0, 4);
        const auto month = digits(5, 2);
        const auto day   = digits(8, 2);
        const auto hour  = has_time ? digits(11, 2) : std::optional(0);
        const auto min   = has_time ? digits(14, 2) : std::optional(0);

        if (!year || !month || !day || !hour || !min) { return std::nullopt; }

        std::tm tm{};
        tm.tm_year  = *year - 1900;
        tm.tm_mon   = *month - 1;
        tm.tm_mday  = *day;
        tm.tm_hour  = *hour;
        tm.tm_min   = *min;
        tm.tm_isdst = -1;

        std::tm start = tm;
        const std::time_t lo = std::mktime(&start);

        // mktime normalises out-of-range fields (2026-02-30 -> 2026-03-02); reject those
        if (lo == -1
            || start.tm_year != tm.tm_year || start.tm_mon != tm.tm_mon || start.tm_mday != tm.tm_mday
            || start.tm_hour != tm.tm_hour || start.tm_min != tm.tm_min)
        {
            return std::nullopt;
        }

        std::tm next = tm;
        if (has_time) { next.tm_min++; } else { next.tm_mday++; }
        next.tm_isdst = -1;

        return Interval{ static_cast<double>(lo), static_cast<double>(std::mktime(&next) - 1) };
    }

    Interval ParseScalar(Kind kind, std::string_view text, Span span)
    {
        if (kind == Kind::Date)
        {
            if (auto date = ParseDate(text)) { return *date; }
            Fail("Expected a date (YYYY-MM-DD or YYYY-MM-DDTHH:MM)", span);
        }

        size_t i = 0;
        while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) { i++; }

        if (i == 0)
        {
            Fail("Expected a " + std::string(KindName(kind)) + " value", span);
        }

        if (i + 1 < text.size() && text[i] == '.' && std::isdigit(static_cast<unsigned char>(text[i + 1])))
        {
            i++;
            while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) { i++; }
        }

        const double number = std::stod(std::string(text.substr(0, i)));
        const auto   unit   = ToLower(text.substr(i));
        const auto   mult   = UnitMultiplier(kind, unit);

        if (!mult.has_value())
        {
            Fail("Unknown " + std::string(KindName(kind)) + " unit '" + unit + "'", span);
        }

        if (kind == Kind::Percent && number > 100)
        {
            Fail("Expected a percentage (0-100)", span);
        }

        const double v = number * *mult;
        return { v, v };
    }

    Filter BuildNumeric(const FieldDef& def, Oper oper, const Value& value)
    {
        const auto& text = value.text;

        if (!value.quoted && text.find(',') != std::string::npos)
        {
            Fail("Lists are not supported for '" + std::string(def.name) + ":'", value.span);
        }

        if (!value.quoted && text.find('*') != std::string::npos)
        {
            Fail("Globs are not supported for '" + std::string(def.name) + ":'", value.span);
        }

        Interval iv{};

        if (const auto dots = text.find(".."); !value.quoted && dots != std::string::npos)
        {
            if (oper != Oper::None)
            {
                Fail("A range can't have an operator", value.span);
            }

            const auto a = ParseScalar(def.kind, std::string_view(text).substr(0, dots), value.span);
            const auto b = ParseScalar(def.kind, std::string_view(text).substr(dots + 2), value.span);

            if (a.lo > b.hi)
            {
                Fail("Range is reversed", value.span);
            }

            iv = { a.lo, b.hi };
        }
        else
        {
            const auto s = ParseScalar(def.kind, text, value.span);

            switch (oper)
            {
            case Oper::None:
            case Oper::Eq:  iv = { s.lo, s.hi }; break;
            case Oper::Gt:  iv = { std::nextafter(s.hi, kInf), kInf }; break;
            case Oper::Gte: iv = { s.lo, kInf }; break;
            case Oper::Lt:  iv = { -kInf, std::nextafter(s.lo, -kInf) }; break;
            case Oper::Lte: iv = { -kInf, s.hi }; break;
            }
        }

        return [get = def.number, iv](const QueryContext& ctx)
        {
            const auto v = get(ctx);
            return v.has_value() && iv.Contains(*v);
        };
    }

    Filter Build(P::OrExprContext* ctx);

    Filter BuildQualifier(P::QualifierExprContext* ctx)
    {
        std::string field = ctx->FIELD()->getText();
        field.pop_back();   // trailing ':'

        std::transform(
            field.begin(),
            field.end(),
            field.begin(),
            [](unsigned char c) { return std::tolower(c); });

        const auto  oper      = OperOf(ctx->op());
        const auto  oper_span = ctx->op() != nullptr ? SpanOf(ctx->op()) : Span{};
              auto* fv        = ctx->fieldValue();
        const auto  value     = ValueOf(fv->V_WORD(), fv->V_STRING());

        if (field == "is" || field == "has")
        {
            if (oper != Oper::None) { Fail("'" + field + ":' does not take an operator", oper_span); }
            return BuildFlag(field, value);
        }

        const auto* def = FindField(field);

        if (def == nullptr)
        {
            Fail("Unknown field '" + field + "'" + Suggest(field), SpanOf(ctx->FIELD()));
        }

        switch (def->kind)
        {
        case Kind::Text: return BuildText(*def, oper, oper_span, value);
        case Kind::Tag:  return BuildTag(oper, oper_span, value);
        case Kind::Hash: return BuildHash(oper, oper_span, value);
        default:         return BuildNumeric(*def, oper, value);
        }
    }

    Filter Build(P::PrimaryContext* ctx)
    {
        if (auto* group = dynamic_cast<P::GroupExprContext*>(ctx))
        {
            return Build(group->orExpr());
        }

        if (auto* qualifier = dynamic_cast<P::QualifierExprContext*>(ctx))
        {
            return BuildQualifier(qualifier);
        }

        auto* text = dynamic_cast<P::TextExprContext*>(ctx);

        return BuildFreeText(ValueOf(text->WORD(), text->STRING()));
    }

    Filter Build(P::UnaryContext* ctx)
    {
        if (auto* negated = dynamic_cast<P::NotExprContext*>(ctx))
        {
            return Not(Build(negated->unary()));
        }

        return Build(dynamic_cast<P::PrimaryExprContext*>(ctx)->primary());
    }

    Filter Build(P::AndExprContext* ctx)
    {
        std::vector<Filter> filters;
        for (auto* unary : ctx->unary()) { filters.push_back(Build(unary)); }
        return All(std::move(filters));
    }

    Filter Build(P::OrExprContext* ctx)
    {
        std::vector<Filter> filters;
        for (auto* and_expr : ctx->andExpr()) { filters.push_back(Build(and_expr)); }
        return Any(std::move(filters));
    }
}

Filter PQL::Parse(std::string_view input)
{
    if (input.size() > kMaxQueryLength)
    {
        throw QueryError(
            "Query too long (max " + std::to_string(kMaxQueryLength) + " characters)",
            kMaxQueryLength,
            input.size());
    }

    ThrowingErrorListener errors;

    antlr4::ANTLRInputStream inputStream(input);
    PqlLexer                 lexer(&inputStream);

    lexer.removeErrorListeners();
    lexer.addErrorListener(&errors);

    antlr4::CommonTokenStream tokens(&lexer);
    tokens.fill();
    CheckDepth(tokens);

    P parser(&tokens);
    parser.removeErrorListeners();
    parser.addErrorListener(&errors);

    auto* query = parser.query();

    if (query->orExpr() == nullptr)
    {
        return [](const QueryContext&) { return true; };
    }

    return Build(query->orExpr());
}
