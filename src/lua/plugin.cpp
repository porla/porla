#include "plugin.hpp"

#include <algorithm>
#include <chrono>
#include <map>
#include <utility>

#include <boost/asio/post.hpp>
#include <boost/log/trivial.hpp>
#include <libtorrent/session_stats.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>

#include "packages/events.hpp"
#include "packages/httpclient.hpp"
#include "packages/httpserver.hpp"
#include "packages/sessions.hpp"
#include "packages/timers.hpp"
#include "pluginsource.hpp"
#include "pluginstate.hpp"
#include "types.hpp"

#include "types/pocancellable.hpp"
#include "types/posessionhandle.hpp"
#include "types/potorrentshandle.hpp"

#include "../config.hpp"
#include "../curlmulti.hpp"
#include "../data/models/sessions.hpp"
#include "../sessions.hpp"
#include "../zip.hpp"

namespace fs = std::filesystem;

using porla::Lua::Plugin;
using porla::Lua::PluginLoadOptions;

namespace
{
    // Formats arguments the way stock print() does - each one run through
    // tostring (so __tostring / __name are honored), separated by tabs.
    std::string Concat(lua_State* L, const sol::variadic_args& args)
    {
        std::string line;

        for (const auto& arg : args)
        {
            std::size_t len = 0;

            // Pushes the string representation; indices in `args` are absolute,
            // so they survive the push.
            const char* str = luaL_tolstring(L, arg.stack_index(), &len);

            if (!line.empty()) line += '\t';
            line.append(str, len);

            lua_pop(L, 1);
        }

        return line;
    }

    std::string DescribeError(const sol::protected_function_result& result)
    {
        if (result.valid() || result.return_count() < 1)
        {
            return "unknown error";
        }

        sol::object value = result.get<sol::object>();

        if (value.is<std::string>())
        {
            return value.as<std::string>();
        }

        return "non-string error value of type "
            + std::string(sol::type_name(result.lua_state(), value.get_type()));
    }
}

struct Plugin::State : public std::enable_shared_from_this<Plugin::State>
{
    PluginLoadOptions           load_options;
    PluginSource                source;
    sol::state                  lua;
    std::shared_ptr<LuaState>   lua_state;
    sol::table                  tbl;
    std::optional<Plugin::Meta> meta;

    State(const PluginLoadOptions& opts)
        : load_options(opts)
    {
        ConfigureLuaState();
    }

    std::string Name() const
    {
        if (meta && meta->name) return *meta->name;
        return "<unnamed plugin>";
    }

    void ConfigureLuaState()
    {
        lua.open_libraries(
            sol::lib::base,
            sol::lib::debug,
            sol::lib::io,
            sol::lib::math,
            sol::lib::os,
            sol::lib::package,
            sol::lib::string,
            sol::lib::table);

        Types::LtAnnounceEndpoint::Register(lua);
        Types::LtAnnounceEntry::Register(lua);
        Types::LtAnnounceInfohash::Register(lua);
        Types::LtOpenFileState::Register(lua);
        Types::LtPeerInfo::Register(lua);
        Types::LtSettingsPack::Register(lua);
        Types::LtTorrentHandle::Register(lua);
        Types::LtTorrentStatus::Register(lua);

        // Porla wrapper types
        Types::PoCancellable::Register(lua);
        Types::PoSessionHandle::Register(lua);
        Types::PoTorrentsHandle::Register(lua);

        lua["package"]["preload"]["porla_events"]      = Packages::Events::Load;
        lua["package"]["preload"]["porla_http_client"] = Packages::HttpClient::Load;
        lua["package"]["preload"]["porla_http_server"] = Packages::HttpServer::Load;
        lua["package"]["preload"]["porla_sessions"]    = Packages::Sessions::Load;
        lua["package"]["preload"]["porla_timers"]      = Packages::Timers::Load;

        lua_state       = std::make_shared<LuaState>(load_options.io, load_options.sessions);
        lua_state->app  = load_options.http_server;
        lua_state->curl = load_options.curl_multi;
        lua_state->db   = load_options.db;

        lua.registry()["state"] = std::weak_ptr(lua_state);

        lua.globals()["print"] = [this](sol::this_state s, sol::variadic_args args)
        {
            BOOST_LOG_TRIVIAL(info) << Name() << ": " << Concat(s, args);
        };
    }
};

std::unique_ptr<Plugin> Plugin::Load(
    const std::filesystem::path& path,
    const std::optional<std::string>& config,
    const PluginLoadOptions& opts)
{
    auto source = PluginSource::Load(path);

    if (!source.has_value())
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load plugin source from " << path;
        return nullptr;
    }

    if (source->sources.find(source->entrypoint) == source->sources.end())
    {
        BOOST_LOG_TRIVIAL(error) << "Plugin entry point (plugin.lua) not found for " << path;
        return nullptr;
    }

    try
    {
        auto state = std::make_unique<State>(opts);
        state->source = *source;

        sol::load_result chunk = state->lua.load_buffer(
            state->source.sources.at(state->source.entrypoint).data(),
            state->source.sources.at(state->source.entrypoint).size(),
            state->source.entrypoint);

        if (!chunk.valid())
        {
            sol::error err = chunk;
            BOOST_LOG_TRIVIAL(error) << "Failed to load plugin: " << err.what();
            return nullptr;
        }

        sol::protected_function_result result = chunk.get<sol::protected_function>()();

        if (!result.valid())
        {
            BOOST_LOG_TRIVIAL(error)
                << "Failed to run plugin: " << DescribeError(result);
            return nullptr;
        }

        if (result.return_count() < 1 || result.get_type() != sol::type::table)
        {
            BOOST_LOG_TRIVIAL(error)
                << "Plugin did not return a table (got "
                << sol::type_name(state->lua.lua_state(), result.get_type()) << ")";
            return nullptr;
        }

        state->meta = Meta{
            .name    = path.filename(),
            .version = std::nullopt
        };

        state->tbl  = result.get<sol::table>();

        if (auto name = state->tbl.get<sol::optional<std::string>>("name"))
        {
            state->meta->name = *name;
        }

        if (auto version = state->tbl.get<sol::optional<std::string>>("version"))
        {
            state->meta->version = *version;
        }

        sol::optional<sol::protected_function> init = state->tbl["init"];

        if (init && init->valid())
        {
            sol::object cfg = sol::nil;

            if (config.has_value())
            {
                sol::load_result config_result = state->lua.load_buffer(
                    config->data(),
                    config->size());

                if (config_result.valid())
                {
                    sol::protected_function config_fn = config_result;
                    sol::protected_function_result config_value = config_fn();

                    if (config_value.valid())
                    {
                        cfg = config_value.get<sol::object>();
                    }
                    else
                    {
                        BOOST_LOG_TRIVIAL(error) << "Failed to evaluate plugin config: " << DescribeError(config_value);
                        return nullptr;
                    }
                }
                else
                {
                    sol::error err = config_result;
                    BOOST_LOG_TRIVIAL(error) << "Failed to parse plugin config: " << err.what();
                    return nullptr;
                }
            }

            sol::protected_function_result init_result = (*init)(cfg);

            if (!init_result.valid())
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to run plugin initializer: " << DescribeError(init_result);
                return nullptr;
            }
        }

        return std::unique_ptr<Plugin>(new Plugin(std::move(state)));
    }
    catch (const std::exception& err)
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
    if (!m_state)
    {
        return;
    }

    sol::optional<sol::protected_function> destroy = m_state->tbl["destroy"];

    if (destroy && destroy->valid())
    {
        (*destroy)();
    }

    for (auto& [ _, cron_schedule ] : m_state->lua_state->cron_schedules)
    {
        cron_schedule->Cancel();
    }

    for (auto& [ _, signal ] : m_state->lua_state->signals)
    {
        signal.disconnect();
    }

    for (auto& [ _, timer ] : m_state->lua_state->timers)
    {
        timer.reset();
    }

    for (auto& dtor : m_state->lua_state->destructors)
    {
        dtor();
    }

    m_state.reset();
}

std::optional<Plugin::Meta> Plugin::GetMeta() const
{
    return m_state ? m_state->meta : std::nullopt;
}
