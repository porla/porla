#include "plugin.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>

#include "globals.hpp"
#include "packages.hpp"
#include "registry.hpp"
#include "types.hpp"

#include "../config.hpp"
#include "../curlmulti.hpp"
#include "../sessions.hpp"
#include "../zip.hpp"

namespace fs = std::filesystem;

using porla::Lua::Plugin;
using porla::Lua::PluginLoadOptions;

struct CoroutineState
{
    sol::thread thread;
};

struct Plugin::State
{
    std::vector<CoroutineState>              active_coroutines;
    boost::asio::steady_timer                active_coroutines_cleanup_timer;
    std::map<std::string, std::vector<char>> files;
    PluginLoadOptions                        load_options;
    sol::state                               lua;
    sol::table                               tbl;
    std::optional<Plugin::Meta>              meta;

    template<typename... Args>
    void SpawnCoroutine(sol::protected_function fn, Args&&... args)
    {
        sol::thread th = sol::thread::create(lua);
        const auto result = sol::coroutine(th.state(), fn)(std::forward<Args>(args)...);
        SpawnCoroutineInternal(std::move(th), result);
    }

    void SpawnCoroutineInternal(sol::thread th, const sol::protected_function_result& result)
    {
        if (!result.valid())
        {
            sol::error err = result;
            BOOST_LOG_TRIVIAL(error) << err.what();
            return;
        }

        if (lua_status(th.state()) != LUA_YIELD)
        {
            return;
        }

        active_coroutines.push_back(CoroutineState{std::move(th)});
    }
};

static sol::table CreatePorlaGlobal(sol::state& lua, std::optional<std::string> config_string)
{
    sol::table porla = lua.create_table();

    porla["config"] = [config_string](sol::this_state s) -> sol::object
    {
        sol::state_view lua{s};

        if (config_string.has_value())
        {
            return lua.script(config_string.value());
        }

        return sol::nil;
    };

    porla["sessions"] = [](sol::this_state s, const std::string& name)
    {
        sol::state_view lua{s};

        auto& sessions = lua.registry()["sessions"].get<porla::Lua::Registry::Sessions>().sessions;

        auto found_session = std::find_if(
            sessions.All().begin(),
            sessions.All().end(),
            [&name](const auto& iter)
            {
                return iter.second->name == name;
            });

        return found_session == sessions.All().end()
            ? nullptr
            : found_session->second;
    };

    return porla;
}

static sol::state CreateLuaState(const PluginLoadOptions& opts, std::optional<std::string> config_string)
{
    sol::state lua;

    lua.open_libraries(
        sol::lib::base,
        sol::lib::io,
        sol::lib::os,
        sol::lib::package,
        sol::lib::string,
        sol::lib::table);

    lua.globals()["http"]      = porla::Lua::Globals::Http::Build(lua);
    lua.globals()["porla"]     = CreatePorlaGlobal(lua, config_string);
    lua.globals()["sleep"]     = porla::Lua::Globals::Sleep::Build(lua);

    lua.registry()["curl"]     = &opts.curl_multi;
    lua.registry()["db"]       = porla::Lua::Registry::Sqlite3{.db = opts.config.db};
    lua.registry()["io"]       = porla::Lua::Registry::BoostIoContext{.io = &opts.io};
    lua.registry()["sessions"] = porla::Lua::Registry::Sessions{.sessions = opts.sessions};

    porla::Lua::Types::LtSettingsPack::Register(lua);
    porla::Lua::Types::Session::Register(lua);

    return lua;
}

std::unique_ptr<Plugin> Plugin::LoadFromArchive(
    const std::vector<char>& buffer,
    const std::optional<std::string>& config,
    const PluginLoadOptions& opts)
{
    return nullptr;
}

std::unique_ptr<Plugin> Plugin::LoadFromPath(
    const std::filesystem::path& path,
    const std::optional<std::string>& config,
    const PluginLoadOptions& opts)
{
    auto state = std::make_unique<State>(State{
        .active_coroutines               = {},
        .active_coroutines_cleanup_timer = boost::asio::steady_timer(opts.io),
        .files                           = {},
        .load_options                    = opts,
        .lua                             = CreateLuaState(opts, config),
        .meta                            = std::nullopt
    });

    try
    {
        const fs::path plugin_lua = fs::is_directory(path)
            ? path / "plugin.lua"
            : path;

        sol::load_result chunk = state->lua.load_file(plugin_lua.string());

        if (!chunk.valid())
        {
            sol::error err = chunk;
            BOOST_LOG_TRIVIAL(error) << "Failed to load plugin: " << err.what();
            return nullptr;
        }

        // Execute on main state — this just builds and returns a table, no yields
        state->tbl = chunk();

        // Read metadata immediately
        state->meta = Plugin::Meta{
            .name    = state->tbl.get<std::string>("name"),
            .version = state->tbl.get<std::string>("version")
        };

        // Run init as a coroutine so it can yield
        sol::optional<sol::protected_function> init = state->tbl["init"];

        if (init)
        {
            state->SpawnCoroutine(*init);
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
    if (m_state->tbl["destroy"].is<sol::function>())
    {
        m_state->tbl["destroy"]();
    }
}

std::optional<Plugin::Meta> Plugin::GetMeta()
{
    return m_state->meta;
}
