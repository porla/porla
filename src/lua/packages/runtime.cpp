#include "runtime.hpp"

using porla::Lua::Packages::Runtime;

sol::object Runtime::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl["api_version_major"] = 1;
    tbl["api_version_minor"] = 0;

    return tbl;
}
