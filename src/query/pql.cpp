#include "pql.hpp"

#include <antlr4-runtime.h>

#include <functional>
#include <utility>
#include <variant>

#include "_aux/PorlaQueryLangBaseVisitor.h"
#include "_aux/PorlaQueryLangLexer.h"
#include "_aux/PorlaQueryLangParser.h"

#include "../torrentclientdata.hpp"
#include "../utils/ratio.hpp"

using porla::Query::PQL;
using porla::Query::QueryError;

typedef std::function<bool(const libtorrent::torrent_status&)> TorrentStatusFilter;
typedef std::variant<std::int64_t, float, std::string>         ValueVariant;

class ExceptionErrorListener : public antlr4::BaseErrorListener
{
public:
    void syntaxError(
        antlr4::Recognizer* /* recognizer */,
        antlr4::Token* /* offendingSymbol */,
        size_t line,
        size_t charPositionInLine,
        const std::string& msg,
        std::exception_ptr e) override {

        e = nullptr;
        std::ostringstream oss;
        oss << "Syntax error - " << msg << " at " << line << ":" << charPositionInLine;
        error_msg = oss.str();

        throw QueryError(msg, charPositionInLine);
    }

private:
    std::string error_msg;
};

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

    throw QueryError("Invalid operator", -1);
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
            {"moving", [](const auto& ts) { return ts.moving_storage; }},
            {"paused", [](const auto& ts) { return (ts.flags & lt::torrent_flags::paused) == lt::torrent_flags::paused; }},
            {"seeding", [](const auto& ts) { return ts.state == lt::torrent_status::seeding; }}
        };

        const auto reference = std::any_cast<std::string>(this->visit(context->reference()));
        const auto flag_ref  = flags_map.find(reference);

        if (flag_ref != flags_map.end())
        {
            return flag_ref->second;
        }

        throw QueryError("Invalid flag '" + reference + "'", context->getStart()->getCharPositionInLine());
    }

    antlrcpp::Any visitFlagExpression(PorlaQueryLangParser::FlagExpressionContext* context) override
    {
        return this->visit(context->flag());
    }

    antlrcpp::Any visitNotFlagExpression(PorlaQueryLangParser::NotFlagExpressionContext* context) override
    {
        const auto flag = std::any_cast<TorrentStatusFilter>(this->visit(context->flag()));
        return TorrentStatusFilter(
            [flag](const lt::torrent_status& ts)
            {
                return !flag(ts);
            });
    }

    antlrcpp::Any visitOperator(PorlaQueryLangParser::OperatorContext* context) override
    {
        if (context->OPER_CONTAINS()) return Oper::CONTAINS;
        if (context->OPER_EQ()) return Oper::EQ;
        if (context->OPER_GT()) return Oper::GT;
        if (context->OPER_GTE()) return Oper::GTE;
        if (context->OPER_LT()) return Oper::LT;
        if (context->OPER_LTE()) return Oper::LTE;
        throw QueryError("Invalid operator: " + context->getText());
    }

    antlrcpp::Any visitOperatorPredicate(PorlaQueryLangParser::OperatorPredicateContext* context) override
    {
        typedef std::function<bool(Oper oper, const ValueVariant&, const lt::torrent_status&)> OperFunc;

        struct OperRef
        {
            OperFunc func;
        };

        static const std::map<std::string, OperRef> oper_map =
        {
            {"active_duration", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (const auto duration = std::get_if<std::int64_t>(&val))
                    {
                        return Compare(ts.active_duration.count(), *duration, oper);
                    }

                    throw QueryError("Invalid value type - expected string");
                }
            }},
            {"age", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (const auto age = std::get_if<std::int64_t>(&val))
                    {
                        const time_t torrent_age = time(nullptr) - ts.added_time;
                        return Compare(torrent_age, *age, oper);
                    }

                    throw QueryError("Invalid value type - expected integer");
                }
            }},
            {"category", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (const auto category = std::get_if<std::string>(&val))
                    {
                        const auto client_data = ts.handle.userdata().get<porla::TorrentClientData>();

                        if (client_data == nullptr || !client_data->category.has_value())
                        {
                            return false;
                        }

                        return Compare(client_data->category.value(), *category, oper);
                    }

                    throw QueryError("Invalid value type - expected string");
                }
            }},
            {"download_rate", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (const auto dl = std::get_if<std::int64_t>(&val))
                    {
                        return Compare(ts.download_rate, *dl, oper);
                    }

                    throw QueryError("Invalid value type - expected number");
                }
            }},
            {"finished_duration", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (const auto duration = std::get_if<std::int64_t>(&val))
                    {
                        return Compare(ts.finished_duration.count(), *duration, oper);
                    }

                    throw QueryError("Invalid value type - expected string");
                }
            }},
            {"name", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (const auto name = std::get_if<std::string>(&val))
                    {
                        if (oper == Oper::CONTAINS)
                        {
                            return ts.name.find(*name) != std::string::npos;
                        }

                        return Compare(ts.name, *name, oper);
                    }

                    throw QueryError("Invalid value type - expected string");
                }
            }},
            {"progress", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (const auto progress = std::get_if<float>(&val))
                    {
                        return Compare(ts.progress, *progress, oper);
                    }

                    throw QueryError("Invalid value type - expected string");
                }
            }},
            {"ratio", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    const auto ratio = porla::Utils::Ratio(ts);

                    if (const auto ratio_int = std::get_if<std::int64_t>(&val))
                    {
                        return Compare(ratio, *ratio_int, oper);
                    }

                    if (const auto ratio_float = std::get_if<float>(&val))
                    {
                        return Compare(ratio, *ratio_float, oper);
                    }

                    throw QueryError("Invalid value type - expected int or float");
                }
            }},
            {"save_path", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (const auto save_path = std::get_if<std::string>(&val))
                    {
                        if (oper == Oper::CONTAINS)
                        {
                            return ts.save_path.find(*save_path) != std::string::npos;
                        }

                        return Compare(ts.save_path, *save_path, oper);
                    }

                    throw QueryError("Invalid value type - expected string");
                }
            }},
            {"seeding_duration", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (const auto duration = std::get_if<std::int64_t>(&val))
                    {
                        return Compare(ts.seeding_duration.count(), *duration, oper);
                    }

                    throw QueryError("Invalid value type - expected string");
                }
            }},
            {"size", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (const auto size = std::get_if<std::int64_t>(&val))
                    {
                        if (const auto torrent_file = ts.torrent_file.lock())
                        {
                            return Compare(torrent_file->total_size(), *size, oper);
                        }

                        return false;
                    }

                    throw QueryError("Invalid value type - expected string");
                }
            }},
            {"tags", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (oper != Oper::CONTAINS)
                    {
                        throw QueryError("tags only support contains");
                    }

                    if (const auto tag = std::get_if<std::string>(&val))
                    {
                        const auto client_data = ts.handle.userdata().get<porla::TorrentClientData>();

                        return client_data != nullptr
                            && client_data->tags.has_value()
                            && client_data->tags->contains(*tag);
                    }

                    throw QueryError("Invalid value type - expected string");
                }
            }},
            {"upload_rate", OperRef{
                .func = [](Oper oper, const ValueVariant& val, const lt::torrent_status& ts)
                {
                    if (const auto ul = std::get_if<std::int64_t>(&val))
                    {
                        return Compare(ts.upload_rate, *ul, oper);
                    }

                    throw QueryError("Invalid value type - expected number");
                }
            }}
        };

        const auto reference = std::any_cast<std::string>(this->visit(context->reference()));
        const auto oper      = std::any_cast<Oper>(this->visit(context->operator_()));
        const auto value     = std::any_cast<ValueVariant>(this->visit(context->value()));

        const auto oper_ref = oper_map.find(reference);

        if (oper_ref == oper_map.end())
        {
            throw QueryError("Invalid reference '" + reference + "'", context->getStart()->getCharPositionInLine());
        }

        return TorrentStatusFilter(
            [oper, value, func = oper_ref->second.func](const lt::torrent_status& ts)
            {
                return func(oper, value, ts);
            });
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
        if (const auto float_value = context->FLOAT())
        {
            return ValueVariant(std::stof(float_value->getText()));
        }

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

            if (auto size = context->UNIT_SIZE())
            {
                if (size->getText() == "kb") val *= 1024;
                if (size->getText() == "mb") val *= 1024 * 1024;
                if (size->getText() == "gb") val *= 1024 * 1024 * 1024;
                if (size->getText() == "tb") val *= 1024 * 1024 * 1024 * 1024l;
                if (size->getText() == "pb") val *= 1024 * 1024 * 1024 * 1024l * 1024l;
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

        throw QueryError("Invalid value type", context->getStart()->getCharPositionInLine());
    }
};

struct PqlFilter : public PQL::Filter
{
    explicit PqlFilter(TorrentStatusFilter filter)
        : m_filter(std::move(filter))
    {
    }

    [[nodiscard]] bool Includes(const libtorrent::torrent_status& ts) const override
    {
        return m_filter(ts);
    }

private:
    TorrentStatusFilter m_filter;
};

std::unique_ptr<PQL::Filter> PQL::Parse(const std::string_view &input)
{
    ExceptionErrorListener errorListener;

    antlr4::ANTLRInputStream inputStream(input);
    PorlaQueryLangLexer lexer{&inputStream};
    lexer.removeErrorListeners();
    lexer.addErrorListener(&errorListener);

    antlr4::CommonTokenStream tokens(&lexer);

    PorlaQueryLangParser parser{&tokens};
    parser.removeErrorListeners();
    parser.addErrorListener(&errorListener);

    Visitor visitor;
    return std::make_unique<PqlFilter>(
        std::any_cast<TorrentStatusFilter>(visitor.visitFilter(parser.filter())));
}
