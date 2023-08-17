#include "plugin.hpp"

#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>

#include "packages.hpp"

namespace fs = std::filesystem;
using porla::Lua::Plugin;
using porla::Lua::PluginLoadOptions;

struct Plugin::State
{
    PluginLoadOptions load_options;
    sol::state        lua;
};

std::unique_ptr<Plugin> Plugin::Load(const PluginLoadOptions& opts)
{
    if (!fs::exists(opts.path))
    {
        BOOST_LOG_TRIVIAL(error) << "Plugin path does not exist - " << opts.path;
        return nullptr;
    }

    const auto plugin_main = fs::is_regular_file(opts.path)
        ? opts.path
        : opts.path / "plugin.lua";

    if (!fs::exists(plugin_main))
    {
        BOOST_LOG_TRIVIAL(error) << "Plugin entry point not found - " << plugin_main;
        return nullptr;
    }

    auto state = std::make_unique<State>(State{
        .load_options = opts,
        .lua          = sol::state()
    });

    state->lua.open_libraries(
        sol::lib::base,
        sol::lib::io,
        sol::lib::os,
        sol::lib::package,
        sol::lib::string,
        sol::lib::table);

    // Put the plugin directory path in the Lua package path
    if (fs::is_directory(opts.path))
    {
        const std::string package_path = state->lua["package"]["path"];
        state->lua["package"]["path"] = package_path + (!package_path.empty() ? ";" : "") + opts.path.string() + "/?.lua";
    }

    state->lua.globals()["__load_opts"] = state->load_options;
    state->lua.globals()["porla"]       = state->lua.create_table();

    Packages::Config::Register(state->lua);
    Packages::Cron::Register(state->lua);
    Packages::Events::Register(state->lua);
    Packages::FileSystem::Register(state->lua);
    Packages::HttpClient::Register(state->lua);
    Packages::Json::Register(state->lua);
    Packages::Log::Register(state->lua);
    Packages::PQL::Register(state->lua);
    Packages::Presets::Register(state->lua);
    Packages::Process::Register(state->lua);
    Packages::Sessions::Register(state->lua);
    Packages::Sqlite::Register(state->lua);
    Packages::Timers::Register(state->lua);
    Packages::Torrents::Register(state->lua);
    Packages::Workflows::Register(state->lua);

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
