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

static std::string ToLower(std::string s)
{
    std::transform(
        s.begin(),
        s.end(),
        s.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    return s;
}

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

static std::string Unquote(std::string text)
{
    if (!text.empty() && text.front() == '"') { text = text.substr(1); }
    if (!text.empty() && text.back()  == '"') { text = text.substr(0, text.size() - 1); }
    return text;
}

class Visitor : public PorlaQueryLangBaseVisitor
{
public:
    // filter : orExpr EOF ;
    antlrcpp::Any visitFilter(PorlaQueryLangParser::FilterContext* context) override
    {
        return this->visit(context->orExpr());
    }

    // orExpr : andExpr (OR andExpr)* ;
    antlrcpp::Any visitOrExpr(PorlaQueryLangParser::OrExprContext* context) override
    {
        std::vector<TorrentStatusFilter> filters;
 
        for (auto* andExpr : context->andExpr())
        {
            filters.emplace_back(std::any_cast<TorrentStatusFilter>(this->visit(andExpr)));
        }
 
        if (filters.size() == 1)
        {
            return filters.front();
        }
 
        return TorrentStatusFilter(
            [filters](const lt::torrent_status& ts)
            {
                return std::any_of(
                    filters.begin(),
                    filters.end(),
                    [&ts](const auto& f) { return f(ts); });
            });
    }

    // andExpr : term (AND? term)* ;
    antlrcpp::Any visitAndExpr(PorlaQueryLangParser::AndExprContext* context) override
    {
        std::vector<TorrentStatusFilter> filters;
 
        for (auto* term : context->term())
        {
            filters.emplace_back(std::any_cast<TorrentStatusFilter>(this->visit(term)));
        }
 
        if (filters.size() == 1)
        {
            return filters.front();
        }
 
        return TorrentStatusFilter(
            [filters](const lt::torrent_status& ts)
            {
                return std::all_of(
                    filters.begin(),
                    filters.end(),
                    [&ts](const auto& f) { return f(ts); });
            });
    }

    // term : NOT term #NotTerm
    antlrcpp::Any visitNotTerm(PorlaQueryLangParser::NotTermContext* context) override
    {
        const auto inner = std::any_cast<TorrentStatusFilter>(this->visit(context->term()));

        return TorrentStatusFilter(
            [inner](const lt::torrent_status& ts)
            {
                return !inner(ts);
            });
    }

    // term : '(' orExpr ')' #GroupTerm
    antlrcpp::Any visitGroupTerm(PorlaQueryLangParser::GroupTermContext* context) override
    {
        return this->visit(context->orExpr());
    }

    // term : qualifier #QualifierTerm
    antlrcpp::Any visitQualifierTerm(PorlaQueryLangParser::QualifierTermContext* context) override
    {
        return this->visit(context->qualifier());
    }
 
    // term : text #TextTerm
    antlrcpp::Any visitTextTerm(PorlaQueryLangParser::TextTermContext* context) override
    {
        return this->visit(context->text());
    }

    // text : STRING | ID | INT | FLOAT ;
    antlrcpp::Any visitText(PorlaQueryLangParser::TextContext* context) override
    {
        std::string needle = context->STRING()
            ? Unquote(context->STRING()->getText())
            : context->getText();

        std::string lower = ToLower(needle);
 
        return TorrentStatusFilter(
            [lower](const lt::torrent_status& ts)
            {
                return ToLower(ts.name).find(lower) != std::string::npos;
            });
    }

    // operator : OPER_EQ | OPER_GT | OPER_GTE | OPER_LT | OPER_LTE ;
    antlrcpp::Any visitOperator(PorlaQueryLangParser::OperatorContext* context) override
    {
        if (context->OPER_EQ())  return Oper::EQ;
        if (context->OPER_GT())  return Oper::GT;
        if (context->OPER_GTE()) return Oper::GTE;
        if (context->OPER_LT())  return Oper::LT;
        if (context->OPER_LTE()) return Oper::LTE;
        throw QueryError("Invalid operator: " + context->getText());
    }

    // qualifier : QUALIFIER operator? value ;
    antlrcpp::Any visitQualifier(PorlaQueryLangParser::QualifierContext* context) override
    {
        // QUALIFIER token includes the trailing ':'
        std::string field = context->QUALIFIER()->getText();
        if (!field.empty() && field.back() == ':') { field.pop_back(); }
 
        const auto value = std::any_cast<ValueVariant>(this->visit(context->value()));
 
        // --- is: flags -------------------------------------------------------
        if (field == "is")
        {
            static const std::map<std::string, TorrentStatusFilter> flags_map =
            {
                {"downloading", [](const auto& ts) { return ts.state == lt::torrent_status::downloading; }},
                {"finished",    [](const auto& ts) { return ts.state == lt::torrent_status::finished; }},
                {"moving",      [](const auto& ts) { return ts.moving_storage; }},
                {"paused",      [](const auto& ts) { return (ts.flags & lt::torrent_flags::paused) == lt::torrent_flags::paused; }},
                {"seeding",     [](const auto& ts) { return ts.state == lt::torrent_status::seeding; }}
            };
 
            const auto state = std::get_if<std::string>(&value);

            if (state == nullptr)
            {
                throw QueryError("is: expects a flag name", context->getStart()->getCharPositionInLine());
            }
 
            const auto flag_ref = flags_map.find(*state);

            if (flag_ref == flags_map.end())
            {
                throw QueryError("Invalid flag '" + *state + "'", context->getStart()->getCharPositionInLine());
            }
 
            return flag_ref->second;
        }
 
        // --- operator: default by value type --------------------------------
        // string value  -> CONTAINS (substring)
        // numeric value -> EQ
        Oper oper;

        if (auto* op = context->operator_())
        {
            oper = std::any_cast<Oper>(this->visit(op));
        }
        else
        {
            oper = std::holds_alternative<std::string>(value) ? Oper::CONTAINS : Oper::EQ;
        }
 
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
 
                    throw QueryError("Invalid value type - expected integer");
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
 
                        // DECISION: category now supports substring (default) like name.
                        // Remove this branch to make category exact-match only.
                        if (oper == Oper::CONTAINS)
                        {
                            return client_data->category.value().find(*category) != std::string::npos;
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
 
                    throw QueryError("Invalid value type - expected integer");
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
 
                    throw QueryError("Invalid value type - expected float");
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
 
                    throw QueryError("Invalid value type - expected integer");
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
 
                    throw QueryError("Invalid value type - expected integer");
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
                            && client_data->tags.contains(*tag);
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
 
        const auto oper_ref = oper_map.find(field);
 
        if (oper_ref == oper_map.end())
        {
            throw QueryError("Invalid reference '" + field + "'", context->getStart()->getCharPositionInLine());
        }
 
        return TorrentStatusFilter(
            [oper, value, func = oper_ref->second.func](const lt::torrent_status& ts)
            {
                return func(oper, value, ts);
            });
    }
 
    // value : (INT | FLOAT) ID? | STRING | ID ;
    antlrcpp::Any visitValue(PorlaQueryLangParser::ValueContext* context) override
    {
        if (const auto float_value = context->FLOAT())
        {
            // NOTE: a unit suffix on a FLOAT (e.g. 1.5gb) is currently ignored.
            return ValueVariant(std::stof(float_value->getText()));
        }
 
        if (const auto int_value = context->INT())
        {
            std::int64_t val = std::stoll(int_value->getText());
 
            if (const auto unit = context->ID())
            {
                static const std::map<std::string, std::int64_t> unit_multipliers =
                {
                    // duration
                    {"s", 1}, {"m", 60}, {"h", 3600}, {"d", 86400}, {"w", 604800},
                    // size
                    {"b", 1}, {"kb", 1024LL}, {"mb", 1024LL * 1024},
                    {"gb", 1024LL * 1024 * 1024}, {"tb", 1024LL * 1024 * 1024 * 1024},
                    {"pb", 1024LL * 1024 * 1024 * 1024 * 1024},
                    // speed
                    {"bps", 1}, {"kbps", 1024LL}, {"mbps", 1024LL * 1024},
                    {"gbps", 1024LL * 1024 * 1024}
                };
 
                const auto mult = unit_multipliers.find(unit->getText());
                if (mult == unit_multipliers.end())
                {
                    throw QueryError(
                        "Unknown unit '" + unit->getText() + "'",
                        context->getStart()->getCharPositionInLine());
                }
 
                val *= mult->second;
            }
 
            return ValueVariant(val);
        }
 
        if (const auto string_value = context->STRING())
        {
            return ValueVariant(Unquote(string_value->getText()));
        }
 
        // bare ID: a word value (e.g. the flag name in `is:downloading`)
        if (const auto id_value = context->ID())
        {
            return ValueVariant(id_value->getText());
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
