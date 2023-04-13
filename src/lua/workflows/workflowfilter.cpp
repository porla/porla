#include "workflowfilter.hpp"

#include "../../query/pql.hpp"

using porla::Lua::Workflows::WorkflowFilter;
using porla::Query::PQL;

bool WorkflowFilter::Includes(const sol::object& filter, const sol::table& context)
{
    if (!filter)
    {
        // No filter was specified
        return false;
    }

    if (filter.is<std::string>())
    {
        const lt::torrent_handle& th = context["lt:torrent_handle"];
        auto pql_filter = PQL::Parse(filter.as<std::string>());
        return pql_filter->Includes(th.status());
    }

    if (filter.is<sol::function>())
    {
        auto func = filter.as<sol::function>();
        auto res = func(context);

        return res;
    }

    return false;
}
