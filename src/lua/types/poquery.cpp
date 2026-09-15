#include "poquery.hpp"

#include "../../query/pql.hpp"

using porla::Lua::Types::PoQuery;

void PoQuery::Register(sol::state& lua)
{
    lua.new_usertype<PoQuery>(
        "PoQuery",
        sol::no_constructor,
        "includes", &PoQuery::Includes,
        "parse", [](const std::string& pql)
        {
            return std::make_shared<PoQuery>(Query::PQL::Parse(pql));
        });
}

PoQuery::PoQuery(const std::function<bool(const lt::torrent_status&)>& filter)
    : m_filter(filter)
{
}

bool PoQuery::Includes(const lt::torrent_status& ts)
{
    return m_filter(ts);
}
