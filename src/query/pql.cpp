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

#include "../fields.hpp"
#include "../torrentclientdata.hpp"
#include "../utils/hex.hpp"
#include "../utils/string.hpp"

using porla::Fields;
using porla::Query::Filter;
using porla::Query::PQL;
using porla::Query::QueryError;
using porla::Utils::String;

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

        return [filters = std::move(filters)](const Fields::Context& ctx)
        {
            return std::all_of(filters.begin(), filters.end(), [&](const auto& f) { return f(ctx); });
        };
    }

    Filter Any(std::vector<Filter> filters)
    {
        if (filters.size() == 1) { return std::move(filters.front()); }

        return [filters = std::move(filters)](const Fields::Context& ctx)
        {
            return std::any_of(filters.begin(), filters.end(), [&](const auto& f) { return f(ctx); });
        };
    }

    Filter Not(Filter inner)
    {
        return [inner = std::move(inner)](const Fields::Context& ctx) { return !inner(ctx); };
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
        return String::Split(value.text, ",");
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

            matchers.push_back({ mode, String::ToLower(item) });
        }

        return matchers;
    }

    // --- registry --------------------------------------------------------------

    std::string Suggest(std::string_view name)
    {
        std::string_view best;
        size_t best_distance = 3;

        for (const auto& field : Fields::All())
        {
            if (const auto d = String::Levenshtein(name, field.name); d < best_distance)
            {
                best          = field.name;
                best_distance = d;
            }
        }

        return best.empty() ? "" : " - did you mean '" + std::string(best) + "'?";
    }

    // --- has: ------------------------------------------------------------

    Filter PresenceOf(const Fields::Field& def, Span span)
    {
        const auto name = std::string(def.name);

        switch (def.kind)
        {
        case Fields::Kind::Tag:
            return [](const Fields::Context& ctx)
            {
                return ctx.client_data != nullptr
                    && !ctx.client_data->tags.empty();
            };

        case Fields::Kind::Text:
            return [get = def.text](const Fields::Context& ctx)
            {
                const auto text = get(ctx);
                return text.has_value() && !text->empty();
            };

        case Fields::Kind::Bool:
            Fail("'" + name + "' always has a value - use '" + name + ":true'", span);

        case Fields::Kind::Flags:
        case Fields::Kind::Hash:
        case Fields::Kind::State:
            Fail("'" + name + "' always has a value; 'has:' can't be used with it", span);

        default:
            return [get = def.number](const Fields::Context& ctx) { return get(ctx).has_value(); };
        }
    }

    Filter BuildHas(Oper oper, Span oper_span, const Value& value)
    {
        if (oper != Oper::None)
        {
            Fail("'has:' does not take an operator", oper_span);
        }

        std::vector<Filter> checks;

        for (const auto& item : ListOf(value))
        {
            const auto name = String::ToLower(item);

            const auto* def = Fields::Find(name);

            if (def == nullptr)
            {
                Fail("Unknown field '" + name + "' for 'has:'" + Suggest(name), value.span);
            }

            checks.push_back(PresenceOf(*def, value.span));
        }

        return Any(std::move(checks));
    }

    // --- text, tag, hash -------------------------------------------------------

    void RequireEqOrNone(Oper oper, Span oper_span, std::string_view what)
    {
        if (oper != Oper::None && oper != Oper::Eq)
        {
            Fail("'" + OperText(oper) + "' is not valid for " + std::string(what), oper_span);
        }
    }

    Filter BuildText(const Fields::Field& def, Oper oper, Span oper_span, const Value& value)
    {
        RequireEqOrNone(oper, oper_span, "text field '" + std::string(def.name) + "'");

        auto matchers = MatchersOf(value, oper == Oper::Eq ? TextMatcher::Mode::Equals : TextMatcher::Mode::Contains);

        return [get = def.text, matchers = std::move(matchers)](const Fields::Context& ctx)
        {
            const auto text = get(ctx);
            if (!text.has_value()) { return false; }

            return std::any_of(matchers.begin(), matchers.end(), [&](const auto& m) { return m.Matches(*text); });
        };
    }

    Filter BuildTag(Oper oper, Span oper_span, const Value& value)
    {
        RequireEqOrNone(oper, oper_span, "field '$userdata.tags'");

        auto matchers = MatchersOf(value, TextMatcher::Mode::Equals);

        return [matchers = std::move(matchers)](const Fields::Context& ctx)
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
        RequireEqOrNone(oper, oper_span, "field 'info_hash'");

        std::vector<std::string> prefixes;

        for (const auto& item : ListOf(value))
        {
            if (item.size() < 4 || item.size() > 64 || !IsHex(item))
            {
                Fail("Expected 4-64 hex characters for 'info_hash:'", value.span);
            }

            prefixes.push_back(String::ToLower(item));
        }

        return [prefixes = std::move(prefixes)](const Fields::Context& ctx)
        {
            return HashHasPrefix(ctx.status.info_hashes, prefixes);
        };
    }

    Filter BuildState(Oper oper, Span oper_span, const Value& value)
    {
        RequireEqOrNone(oper, oper_span, "field 'state'");

        auto unparsed_states = ListOf(value);

        std::vector<lt::torrent_status::state_t> states;

        std::transform(
            unparsed_states.begin(),
            unparsed_states.end(),
            std::back_inserter(states),
            [&](const std::string& s)
            {
                const auto lower = String::ToLower(s);
                if (lower == "checking_files")       return lt::torrent_status::checking_files;
                if (lower == "downloading_metadata") return lt::torrent_status::downloading_metadata;
                if (lower == "downloading")          return lt::torrent_status::downloading;
                if (lower == "finished")             return lt::torrent_status::finished;
                if (lower == "seeding")              return lt::torrent_status::seeding;
                if (lower == "checking_resume_data") return lt::torrent_status::checking_resume_data;
                Fail("Unknown torrent state '" + s + "'", value.span);
            });

        return [states = std::move(states)](const Fields::Context& ctx)
        {
            return std::any_of(
                states.begin(),
                states.end(),
                [&](const auto& s)
                {
                    return ctx.status.state == s;
                });
        };
    }

    Filter BuildBool(const Fields::Field& field, Oper oper, Span oper_span, const Value& value)
    {
        RequireEqOrNone(oper, oper_span, "bool field '" + std::string(field.name) + "'");

        static const std::map<std::string, bool> bools =
        {
            {"true", true},
            {"1", true},
            {"yes", true},
            {"false", false},
            {"0", false},
            {"no", false}
        };

        const auto val = String::ToLower(value.text);
        const auto it  = bools.find(val);

        if (it == bools.end())
        {
            Fail("Unexpected value for boolean", value.span);
        }

        return [get = field.boolean, v = it->second](const Fields::Context& ctx)
        {
            return get(ctx) == v;
        };
    }

    Filter BuildFlags(Oper oper, Span oper_span, const Value& value)
    {
        if (oper != Oper::None)
        {
            Fail("'" + OperText(oper) + "' is not valid for field 'flags'", oper_span);
        }

        static const std::map<std::string_view, lt::torrent_flags_t> names =
        {
            { "apply_ip_filter",       lt::torrent_flags::apply_ip_filter },
            { "auto_managed",          lt::torrent_flags::auto_managed },
            { "default_dont_download", lt::torrent_flags::default_dont_download },
            { "disable_dht",           lt::torrent_flags::disable_dht },
            { "disable_lsd",           lt::torrent_flags::disable_lsd },
            { "disable_pex",           lt::torrent_flags::disable_pex },
            { "disable_v1_hashes",     lt::torrent_flags::disable_v1_hashes },
            { "duplicate_is_error",    lt::torrent_flags::duplicate_is_error },
            { "i2p_torrent",           lt::torrent_flags::i2p_torrent },
            { "need_save_resume",      lt::torrent_flags::need_save_resume },
            { "no_verify_files",       lt::torrent_flags::no_verify_files },
            { "paused",                lt::torrent_flags::paused },
            { "seed_mode",             lt::torrent_flags::seed_mode },
            { "sequential_download",   lt::torrent_flags::sequential_download },
            { "share_mode",            lt::torrent_flags::share_mode },
            { "stop_when_ready",       lt::torrent_flags::stop_when_ready },
            { "super_seeding",         lt::torrent_flags::super_seeding },
            { "update_subscribe",      lt::torrent_flags::update_subscribe },
            { "upload_mode",           lt::torrent_flags::upload_mode },
        };

        lt::torrent_flags_t mask{};
        lt::torrent_flags_t want{};

        for (const auto& item : ListOf(value))
        {
            const bool negate = item.starts_with('~');
            const auto name   = String::ToLower(std::string_view(item).substr(negate ? 1 : 0));
            const auto it     = names.find(name);

            if (it == names.end())
            {
                std::string_view best;
                size_t best_distance = 3;

                for (const auto& [candidate, _] : names)
                {
                    if (const auto d = String::Levenshtein(name, candidate); d < best_distance)
                    {
                        best          = candidate;
                        best_distance = d;
                    }
                }

                Fail(
                    "Unknown flag '" + name + "' for 'flags:'"
                        + (best.empty() ? "" : " - did you mean '" + std::string(best) + "'?"),
                    value.span);
            }

            if (mask & it->second)
            {
                Fail("Flag '" + name + "' is given more than once for 'flags:'", value.span);
            }

            mask |= it->second;

            if (!negate)
            {
                want |= it->second;
            }
        }

        return [mask, want](const Fields::Context& ctx)
        {
            return (ctx.status.flags & mask) == want;
        };
    }

    Filter BuildFreeText(const Value& value)
    {
        const auto lower   = String::ToLower(value.text);
        const bool as_hash = !value.quoted && LooksLikeHash(lower);

        return [lower, as_hash, prefixes = std::vector<std::string>{ lower }](const Fields::Context& ctx)
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
        bool   lo_relative = false;
        bool   hi_relative = false;
        bool   lo_open     = false;
        bool   hi_open     = false;

        [[nodiscard]] double Lo(std::time_t now) const
        {
            return lo_relative ? static_cast<double>(now) + lo : lo;
        }

        [[nodiscard]] double Hi(std::time_t now) const
        {
            return hi_relative ? static_cast<double>(now) + hi : hi;
        }

        [[nodiscard]] bool Contains(double v, std::time_t now) const
        {
            const double l = Lo(now);
            const double h = Hi(now);

            return (lo_open ? v > l : v >= l)
                && (hi_open ? v < h : v <= h);
        }
    };

    constexpr double kInf = std::numeric_limits<double>::infinity();

    std::string_view KindName(Fields::Kind kind)
    {
        switch (kind)
        {
        case Fields::Kind::Size:     return "size";
        case Fields::Kind::Rate:     return "rate";
        case Fields::Kind::Duration: return "duration";
        case Fields::Kind::Percent:  return "percent";
        case Fields::Kind::Date:     return "date";
        default:                     return "number";
        }
    }

    std::optional<double> UnitMultiplier(Fields::Kind kind, std::string_view unit)
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
        case Fields::Kind::Size:     return find(sizes, unit);
        case Fields::Kind::Rate:     return find(sizes, unit.ends_with("/s") ? unit.substr(0, unit.size() - 2) : unit);
        case Fields::Kind::Duration: return find(durations, unit);
        case Fields::Kind::Percent:  return unit.empty() || unit == "%" ? std::optional(10000.) : std::nullopt;   // percent -> ppm
        default:                     return unit.empty() ? std::optional(1.) : std::nullopt;
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

    // Length of the leading "123" or "1.5" in text, or 0 if it doesn't start with a number.
    size_t NumberPrefix(std::string_view text)
    {
        size_t i = 0;
        while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) { i++; }

        if (i > 0 && i + 1 < text.size() && text[i] == '.' && std::isdigit(static_cast<unsigned char>(text[i + 1])))
        {
            i++;
            while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) { i++; }
        }

        return i;
    }

    // "1h", "2w", "1.5d" - a number with an explicit duration unit
    bool IsDurationWithUnit(std::string_view text)
    {
        const auto n = NumberPrefix(text);
        if (n == 0 || n == text.size()) { return false; }

        return UnitMultiplier(Fields::Kind::Duration, String::ToLower(text.substr(n))).has_value();
    }

    Interval ParseScalar(Fields::Kind kind, std::string_view text, Span span)
    {
        if (kind == Fields::Kind::Date)
        {
            if (text.starts_with("-"))
            {
                const auto ago = ParseScalar(Fields::Kind::Duration, text.substr(1), span);

                return {
                    .lo          = -ago.lo,
                    .hi          = -ago.hi,
                    .lo_relative = true,
                    .hi_relative = true
                };
            }

            if (auto date = ParseDate(text))
            {
                return *date;
            }

            if (IsDurationWithUnit(text))
            {
                Fail("Expected a date or a relative time - did you mean '-" + std::string(text) + "'?", span);
            }

            Fail("Expected a date (YYYY-MM-DD or YYYY-MM-DDTHH:MM) or a relative time (-1w)", span);
        }

        const size_t i = NumberPrefix(text);

        if (i == 0)
        {
            Fail("Expected a " + std::string(KindName(kind)) + " value", span);
        }

        double number = 0;
        try { number = std::stod(std::string(text.substr(0, i))); }
        catch (const std::out_of_range&) { Fail("Number is out of range", span); }

        const auto   unit   = String::ToLower(text.substr(i));
        const auto   mult   = UnitMultiplier(kind, unit);

        if (!mult.has_value())
        {
            Fail("Unknown " + std::string(KindName(kind)) + " unit '" + unit + "'", span);
        }

        if (kind == Fields::Kind::Percent && number > 100)
        {
            Fail("Expected a percentage (0-100)", span);
        }

        const double v = number * *mult;
        return { v, v };
    }

    Filter BuildNumeric(const Fields::Field& def, Oper oper, Span oper_span, const Value& value)
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

            const auto check_now = std::time(nullptr);

            if (a.Lo(check_now) > b.Hi(check_now))
            {
                Fail("Range is reversed", value.span);
            }

            iv = {
                .lo          = a.lo,
                .hi          = b.hi,
                .lo_relative = a.lo_relative,
                .hi_relative = b.hi_relative
            };
        }
        else
        {
            const auto s = ParseScalar(def.kind, text, value.span);

            const bool rel = s.lo_relative;

            if (rel && oper == Oper::Eq)
            {
                Fail("'=' is not valid with a relative time; use '>' or '<'", oper_span);
            }

            switch (oper)
            {
            case Oper::None:
                // a bare relative time means "within the last ...", i.e. >=
                iv = rel ? Interval{ .lo = s.lo, .hi = kInf, .lo_relative = true } : s;
                break;
            case Oper::Eq:  iv = s; break;
            case Oper::Gt:  iv = { .lo = s.hi, .hi = kInf, .lo_relative = rel, .lo_open = true }; break;
            case Oper::Gte: iv = { .lo = s.lo, .hi = kInf, .lo_relative = rel }; break;
            case Oper::Lt:  iv = { .lo = -kInf, .hi = s.lo, .hi_relative = rel, .hi_open = true }; break;
            case Oper::Lte: iv = { .lo = -kInf, .hi = s.hi, .hi_relative = rel }; break;            }
        }

        return [get = def.number, iv](const Fields::Context& ctx)
        {
            const auto v = get(ctx);
            return v.has_value() && iv.Contains(*v, ctx.now);
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

        if (field == "has")
        {
            return BuildHas(oper, oper_span, value);
        }

        const auto* def = Fields::Find(field);

        if (def == nullptr)
        {
            Fail("Unknown field '" + field + "'" + Suggest(field), SpanOf(ctx->FIELD()));
        }

        switch (def->kind)
        {
        case Fields::Kind::Text:  return BuildText(*def, oper, oper_span, value);
        case Fields::Kind::Tag:   return BuildTag(oper, oper_span, value);
        case Fields::Kind::Hash:  return BuildHash(oper, oper_span, value);
        case Fields::Kind::State: return BuildState(oper, oper_span, value);
        case Fields::Kind::Bool:  return BuildBool(*def, oper, oper_span, value);
        case Fields::Kind::Flags: return BuildFlags(oper, oper_span, value);
        default:                  return BuildNumeric(*def, oper, oper_span, value);
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
        return [](const Fields::Context&) { return true; };
    }

    return Build(query->orExpr());
}
