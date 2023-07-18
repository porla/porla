#include "json.hpp"

static const char json_lua[] =
#include "_/json.lua"
;

using porla::Lua::Packages::Json;

void Json::Register(sol::state& lua)
{
    lua["package"]["preload"]["json"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        return lua.script(json_lua, "json.lua");
    };
}
