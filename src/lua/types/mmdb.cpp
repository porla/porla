#include "../types.hpp"

#include <zip.h>

using porla::Lua::Types::Mmdb;

void Mmdb::Register(sol::state& lua)
{
    sol::table mmdb = lua["mmdb"].valid()
        ? lua["mmdb"].get<sol::table>()
        : lua.create_named_table("mmdb");

}
