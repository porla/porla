#include "poerror.hpp"

using porla::Lua::Types::PoError;

sol::object PoError::Construct(sol::this_state ts, const lt::error_code& ec)
{
    if (!ec)
    {
        return sol::nil;
    }

    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();
    tbl["category"] = ec.category().name();
    tbl["message"]  = std::string(ec.message());
    tbl["value"]    = ec.value();

    return tbl;
}
