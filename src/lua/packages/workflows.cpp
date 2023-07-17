#include "workflows.hpp"

#include "../plugins/plugin.hpp"

static const char workflows_lua[] =
#include "_/workflows.lua"
;

static const char workflows_actions_exec_lua[] =
#include "_/workflows.actions.exec.lua"
;

static const char workflows_actions_http_lua[] =
#include "_/workflows.actions.http.lua"
;

static const char workflows_actions_log_lua[] =
#include "_/workflows.actions.log.lua"
;

static const char workflows_actions_notify_lua[] =
#include "_/workflows.actions.notify.lua"
;

static const char workflows_actions_sleep_lua[] =
#include "_/workflows.actions.sleep.lua"
;


static const char workflows_actions_torrent_lua[] =
#include "_/workflows.actions.torrent.lua"
;

static const char workflows_triggers_cron_lua[] =
#include "_/workflows.triggers.cron.lua"
;

static const char workflows_triggers_interval_lua[] =
#include "_/workflows.triggers.interval.lua"
;

static const char workflows_triggers_torrent_lua[] =
#include "_/workflows.triggers.torrent.lua"
;

using porla::Lua::Packages::Workflows;
using porla::Lua::Plugins::PluginLoadOptions;

void Workflows::Register(sol::state& lua)
{
    lua["package"]["preload"]["workflows"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        return lua.script(workflows_lua, "workflows.lua");
    };

    lua["package"]["preload"]["workflows.actions.exec"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        return lua.script(workflows_actions_exec_lua, "workflows.actions.exec.lua");
    };

    lua["package"]["preload"]["workflows.actions.http"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        return lua.script(workflows_actions_http_lua, "workflows.actions.http.lua");
    };

    lua["package"]["preload"]["workflows.actions.log"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        return lua.script(workflows_actions_log_lua, "workflows.actions.log.lua");
    };

    lua["package"]["preload"]["workflows.actions.notify"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        return lua.script(workflows_actions_notify_lua, "workflows.actions.notify.lua");
    };

    lua["package"]["preload"]["workflows.actions.sleep"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        return lua.script(workflows_actions_sleep_lua, "workflows.actions.sleep.lua");
    };

    lua["package"]["preload"]["workflows.actions.torrent"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        return lua.script(workflows_actions_torrent_lua, "workflows.actions.torrent.lua");
    };

    lua["package"]["preload"]["workflows.triggers.cron"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        return lua.script(workflows_triggers_cron_lua, "workflows.triggers.cron.lua");
    };

    lua["package"]["preload"]["workflows.triggers.interval"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        return lua.script(workflows_triggers_interval_lua, "workflows.triggers.interval.lua");
    };

    lua["package"]["preload"]["workflows.triggers.torrent"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        return lua.script(workflows_triggers_torrent_lua, "workflows.triggers.torrent.lua");
    };
}
