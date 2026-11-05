#include "poquery.hpp"

#include "poerror.hpp"

#include "../../fields.hpp"
#include "../../query/pql.hpp"

using porla::Lua::Types::PoQuery;

void PoQuery::Register(sol::state& lua)
{
    lua.new_usertype<PoQuery>(
        "PoQuery",
        sol::no_constructor,
        "includes", &PoQuery::Includes,
        "parse", [](sol::this_state ts, const std::string& pql) -> std::tuple<sol::object, sol::object>
        {
            try
            {
                return { sol::make_object(ts, std::make_shared<PoQuery>(Query::PQL::Parse(pql))), sol::lua_nil };
            }
            catch (const std::exception& e)
            {
                return { sol::lua_nil, PoError::Make(ts, porla::Errors::Code::invalid_params, e.what()) };
            }
        });
}

PoQuery::PoQuery(const Query::Filter& filter)
    : m_filter(filter)
{
}

bool PoQuery::Includes(const lt::torrent_status& ts)
{
    const Fields::Context ctx{
        .status      = ts,
        .client_data = ts.handle.userdata().get<TorrentClientData>(),
        .now         = std::time(nullptr)
    };

    return m_filter(ctx);
}
