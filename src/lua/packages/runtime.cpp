#include "../packages.hpp"

#include "../pluginstate.hpp"
#include "../../buildinfo.hpp"
#include "../../config.hpp"

using porla::Lua::Packages::Runtime;

sol::object Runtime::Load(sol::this_state ts)
{
    auto state = LuaState::From(ts);
    if (state == nullptr) { return sol::lua_nil; }
    
    sol::state_view lua(ts);
    sol::table tbl = lua.create_table();

    sol::table args = lua.create_table();
    args["http-base-path"] = state->cfg.http_base_path;
    args["state-dir"]      = state->cfg.state_dir.string();

    tbl["api_version_major"] = 1;
    tbl["api_version_minor"] = 0;
    tbl["args"]              = args;
    tbl["version"]           = BuildInfo::Version();

    return tbl;
}
