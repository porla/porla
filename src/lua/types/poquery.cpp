#include "poquery.hpp"

#include "../../query/pql.hpp"

using porla::Lua::Types::PoQuery;

void PoQuery::Register(sol::state& lua)
{
    lua.new_usertype<PoQuery>(
        "PoQuery",
        sol::no_constructor,
        "includes", &PoQuery::Includes,
        "parse", [](const std::string& pql) -> std::tuple<std::shared_ptr<PoQuery>, std::optional<std::string>>
        {
            try
            {
                auto query = Query::PQL::Parse(pql);
                return std::make_tuple(
                    std::make_shared<PoQuery>(query),
                    std::nullopt);
            }
            catch(const std::exception& e)
            {
                return std::make_tuple(nullptr, e.what());
            }
        });
}

PoQuery::PoQuery(const Query::Filter& filter)
    : m_filter(filter)
{
}

bool PoQuery::Includes(const lt::torrent_status& ts)
{
    const Query::QueryContext ctx{
        .status      = ts,
        .client_data = ts.handle.userdata().get<TorrentClientData>(),
        .now         = std::time(nullptr)
    };

    return m_filter(ctx);
}
