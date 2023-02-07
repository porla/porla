#include "pql.hpp"

#include <antlr4-runtime.h>

#include <functional>
#include <utility>
#include <variant>

#include "_aux/PorlaQueryLangBaseVisitor.h"
#include "_aux/PorlaQueryLangLexer.h"
#include "_aux/PorlaQueryLangParser.h"

using porla::Query::PQL;

typedef std::function<bool(const libtorrent::torrent_status&)> TorrentStatusFilter;
typedef std::variant<std::int64_t, float, std::string>         ValueVariant;

enum class Oper
{
    CONTAINS,
    EQ,
    GT,
    GTE,
    LT,
    LTE
};

template<typename TLeft, typename TRight>
bool Compare(TLeft const& lhs, TRight const& rhs, Oper oper)
{
    switch (oper)
    {
        case Oper::LT: return lhs < rhs;
        case Oper::LTE: return lhs <= rhs;
        case Oper::EQ: return lhs == rhs;
        case Oper::GT: return lhs > rhs;
        case Oper::GTE: return lhs >= rhs;
        case Oper::CONTAINS:
            break;
    }

    throw std::runtime_error("Unknown operator");
}

class Visitor : public PorlaQueryLangBaseVisitor
{
public:
    antlrcpp::Any visitAndExpression(PorlaQueryLangParser::AndExpressionContext* context) override
    {
        std::vector<TorrentStatusFilter> filters;

        for (const auto expression : context->expression())
        {
            filters.emplace_back(std::any_cast<TorrentStatusFilter>(this->visit(expression)));
        }

        return TorrentStatusFilter(
            [filters](const libtorrent::torrent_status& ts)
            {
                return std::all_of(
                    filters.begin(),
                    filters.end(),
                    [&ts](const auto& f) { return f(ts); });
            });
    }

    antlrcpp::Any visitFilter(PorlaQueryLangParser::FilterContext* context) override
    {
        return this->visit(context->expression());
    }

    antlrcpp::Any visitFlag(PorlaQueryLangParser::FlagContext* context) override
    {
        static const std::map<std::string, TorrentStatusFilter> flags_map =
        {
            {"downloading", [](const auto& ts) { return ts.state == lt::torrent_status::downloading; }},
            {"finished", [](const auto& ts) { return ts.state == lt::torrent_status::finished; }},
            {"paused", [](const auto& ts) { return (ts.flags & lt::torrent_flags::paused) == lt::torrent_flags::paused; }},
            {"seeding", [](const auto& ts) { return ts.state == lt::torrent_status::seeding; }}
        };

        const auto reference = std::any_cast<std::string>(this->visit(context->reference()));
        const auto flag_ref  = flags_map.find(reference);

        if (flag_ref != flags_map.end())
        {
            return flag_ref->second;
        }

        throw std::runtime_error("invalid flag");
    }

    antlrcpp::Any visitFlagExpression(PorlaQueryLangParser::FlagExpressionContext* context) override
    {
        return this->visit(context->flag());
    }

    antlrcpp::Any visitOperator(PorlaQueryLangParser::OperatorContext* context) override
    {
        if (context->OPER_CONTAINS()) return Oper::CONTAINS;
        if (context->OPER_EQ()) return Oper::EQ;
        if (context->OPER_GT()) return Oper::GT;
        if (context->OPER_GTE()) return Oper::GTE;
        if (context->OPER_LT()) return Oper::LT;
        if (context->OPER_LTE()) return Oper::LTE;
        throw std::runtime_error("invalid operator");
    }

    antlrcpp::Any visitOperatorPredicate(PorlaQueryLangParser::OperatorPredicateContext* context) override
    {
        const auto reference = std::any_cast<std::string>(this->visit(context->reference()));
        const auto oper      = std::any_cast<Oper>(this->visit(context->operator_()));
        const auto value     = std::any_cast<ValueVariant>(this->visit(context->value()));

        if (reference == "age")
        {
            return TorrentStatusFilter(
                [val = std::get<std::int64_t>(value), oper](const libtorrent::torrent_status& ts)
                {
                    const time_t age = time(nullptr) - ts.added_time;
                    return Compare(age, val, oper);
                });
        }

        if (reference == "category")
        {
            return TorrentStatusFilter(
                [](const libtorrent::torrent_status& ts)
                {
                    return false;
                });
        }

        if (reference == "dl")
        {
            return TorrentStatusFilter(
                [val = std::get<std::int64_t>(value), oper](const libtorrent::torrent_status& ts)
                {
                    return Compare(ts.download_payload_rate, val, oper);
                });
        }

        if (reference == "name")
        {
            return TorrentStatusFilter(
                [val = std::get<std::string>(value)](const libtorrent::torrent_status& ts)
                {
                    return ts.name.find(val) != std::string::npos;
                });
        }

        if (reference == "size")
        {
            return TorrentStatusFilter(
                [](const libtorrent::torrent_status& ts)
                {
                    return false;
                });
        }

        if (reference == "tags")
        {
            return TorrentStatusFilter(
                [](const libtorrent::torrent_status& ts)
                {
                    return false;
                });
        }

        if (reference == "ul")
        {
            return TorrentStatusFilter(
                [val = std::get<std::int64_t>(value), oper](const libtorrent::torrent_status& ts)
                {
                    return Compare(ts.upload_payload_rate, val, oper);
                });
        }

        throw std::runtime_error("invalid operator predicate");
    }

    antlrcpp::Any visitOrExpression(PorlaQueryLangParser::OrExpressionContext* context) override
    {
        std::vector<TorrentStatusFilter> filters;

        for (const auto expression : context->expression())
        {
            filters.emplace_back(std::any_cast<TorrentStatusFilter>(this->visit(expression)));
        }

        return TorrentStatusFilter(
            [filters](const libtorrent::torrent_status& ts)
            {
                return std::any_of(
                    filters.begin(),
                    filters.end(),
                    [&ts](const auto& f) { return f(ts); });
            });
    }

    antlrcpp::Any visitPredicateExpression(PorlaQueryLangParser::PredicateExpressionContext* context) override
    {
        return this->visit(context->predicate());
    }

    antlrcpp::Any visitReference(PorlaQueryLangParser::ReferenceContext* context) override
    {
        return context->getText();
    }

    antlrcpp::Any visitValue(PorlaQueryLangParser::ValueContext* context) override
    {
        if (const auto int_value = context->INT())
        {
            std::int64_t val = std::stoll(int_value->getText());

            if (auto duration = context->UNIT_DURATION())
            {
                if (duration->getText() == "m") val *= 60;
                if (duration->getText() == "h") val *= 60 * 60;
                if (duration->getText() == "d") val *= 60 * 60 * 24;
                if (duration->getText() == "w") val *= 60 * 60 * 24 * 7;
            }

            if (auto speed = context->UNIT_SPEED())
            {
                if (speed->getText() == "kbps") val *= 1024;
                if (speed->getText() == "mbps") val *= 1024 * 1024;
                if (speed->getText() == "gbps") val *= 1024 * 1024 * 1024;
            }

            return ValueVariant(val);
        }

        if (const auto string_value = context->STRING())
        {
            std::string text = string_value->getText();
            if (!text.empty() && text[0] == '\"') { text = text.substr(1); }
            if (!text.empty() && text[text.size() - 1] == '\"') { text = text.substr(0, text.size() - 1); }

            return ValueVariant(text);
        }

        throw std::runtime_error("Invalid value");
    }
};

struct PqlFilter : public PQL::Filter
{
    explicit PqlFilter(TorrentStatusFilter filter)
        : m_filter(std::move(filter))
    {
    }

    bool Includes(const libtorrent::torrent_status& ts) override
    {
        return m_filter(ts);
    }

private:
    TorrentStatusFilter m_filter;
};

std::unique_ptr<PQL::Filter> PQL::Parse(const std::string_view &input)
{
    antlr4::ANTLRInputStream inputStream(input);
    PorlaQueryLangLexer lexer{&inputStream};
    antlr4::CommonTokenStream tokens(&lexer);

    PorlaQueryLangParser parser{&tokens};

    Visitor visitor;

    return std::make_unique<PqlFilter>(
        std::any_cast<TorrentStatusFilter>(visitor.visitFilter(parser.filter())));
}
