#include "plugin.hpp"

#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>

#include "../packages/config.hpp"
#include "../packages/cron.hpp"
#include "../packages/filesystem.hpp"
#include "../packages/log.hpp"
#include "../packages/sqlite.hpp"
#include "../packages/torrents.hpp"

namespace fs = std::filesystem;
using porla::Lua::Plugins::Plugin;
using porla::Lua::Plugins::PluginLoadOptions;

struct Plugin::State
{
    PluginLoadOptions load_options;
    sol::state        lua;
};

std::unique_ptr<Plugin> Plugin::Load(const PluginLoadOptions& opts)
{
    if (!fs::exists(opts.dir))
    {
        BOOST_LOG_TRIVIAL(error) << "Plugin directory does not exist - " << opts.dir;
        return nullptr;
    }

    const auto plugin_main = opts.dir / "plugin.lua";

    if (!fs::exists(plugin_main))
    {
        BOOST_LOG_TRIVIAL(error) << "No 'plugin.lua' found in directory - " << opts.dir;
        return nullptr;
    }

    auto state = std::make_unique<State>(State{
        .load_options = opts,
        .lua          = sol::state()
    });

    state->lua.open_libraries(
        sol::lib::base,
        sol::lib::package,
        sol::lib::string);

    // Put the plugin directory path in the Lua package path
    const std::string package_path = state->lua["package"]["path"];
    state->lua["package"]["path"] = package_path + (!package_path.empty() ? ";" : "") + opts.dir.string() + "/?.lua";

    state->lua.globals()["__load_opts"] = state->load_options;
    state->lua.globals()["porla"]       = state->lua.create_table();

    Packages::Config::Register(state->lua);
    Packages::Cron::Register(state->lua);
    Packages::FileSystem::Register(state->lua);
    Packages::Log::Register(state->lua);
    Packages::Sqlite::Register(state->lua);
    Packages::Torrents::Register(state->lua);

    try
    {
        state->lua.script_file(plugin_main.string());

        if (state->lua.globals()["porla"]["init"].is<sol::function>())
        {
            state->lua.globals()["porla"]["init"]();
        }

        return std::unique_ptr<Plugin>(new Plugin(std::move(state)));
    }
    catch (const sol::error& err)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load plugin: " << err.what();
    }

    return nullptr;
}

Plugin::Plugin(std::unique_ptr<State> state)
    : m_state(std::move(state))
{
}

Plugin::~Plugin()
{
    if (m_state->lua.globals()["porla"]["destroy"].is<sol::function>())
    {
        m_state->lua.globals()["porla"]["destroy"]();
    }
}
