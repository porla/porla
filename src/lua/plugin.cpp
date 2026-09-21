#include "plugin.hpp"

#include <algorithm>
#include <chrono>
#include <map>
#include <utility>

#include <boost/asio/post.hpp>
#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>

#include "packages/codec.hpp"
#include "packages/crypto.hpp"
#include "packages/events.hpp"
#include "packages/httpclient.hpp"
#include "packages/httpserver.hpp"
#include "packages/presets.hpp"
#include "packages/runtime.hpp"
#include "packages/sessions.hpp"
#include "packages/sockets.hpp"
#include "packages/timers.hpp"
#include "pluginsource.hpp"
#include "pluginstate.hpp"
#include "print.hpp"
#include "types.hpp"

#include "types/ltaddtorrentparams.hpp"
#include "types/pocancellable.hpp"
#include "types/pohttpserverresponse.hpp"
#include "types/popreset.hpp"
#include "types/poquery.hpp"
#include "types/posessionhandle.hpp"
#include "types/potcpclient.hpp"
#include "types/potorrentdata.hpp"
#include "types/potorrentshandle.hpp"
#include "types/sohashstate.hpp"

#include "../config.hpp"
#include "../curlmulti.hpp"
#include "../sessions.hpp"

namespace fs = std::filesystem;

using porla::Lua::Plugin;
using porla::Lua::PluginLoadOptions;

namespace
{
    static int PORLA_LUA_API_VERSION = 1;

    std::string DescribeError(const sol::protected_function_result& result)
    {
        if (result.valid() || result.return_count() < 1)
        {
            return "unknown error";
        }

        sol::error err = result;

        return err.what();
    }
}

struct Plugin::State
{
    PluginLoadOptions           load_options;
    PluginSource                source;
    sol::state                  lua;
    std::shared_ptr<LuaState>   lua_state;
    sol::table                  tbl;

    State(const PluginLoadOptions& opts)
        : load_options(opts)
    {
        ConfigureLuaState();
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

        Types::LtAddTorrentParams::Register(lua);
        Types::LtAnnounceEndpoint::Register(lua);
        Types::LtAnnounceEntry::Register(lua);
        Types::LtAnnounceInfohash::Register(lua);
        Types::LtInfoHash::Register(lua);
        Types::LtOpenFileState::Register(lua);
        Types::LtPeerInfo::Register(lua);
        Types::LtSettingsPack::Register(lua);
        Types::LtTorrentFlags::Register(lua);
        Types::LtTorrentInfo::Register(lua);
        Types::LtTorrentHandle::Register(lua);
        Types::LtTorrentStatus::Register(lua);

        // Porla wrapper types
        Types::PoCancellable::Register(lua);
        Types::PoHttpServerResponse::Register(lua);
        Types::PoPreset::Register(lua);
        Types::PoQuery::Register(lua);
        Types::PoSessionHandle::Register(lua);
        Types::PoTcpClient::Register(lua);
        Types::PoTorrentsHandle::Register(lua);
        Types::PoTorrentData::Register(lua);
        Types::SoHashState::Register(lua);

        sol::table package = lua["package"];
        package["preload"]["porla_codec"]       = Packages::Codec::Load;
        package["preload"]["porla_crypto"]      = Packages::Crypto::Load;
        package["preload"]["porla_events"]      = Packages::Events::Load;
        package["preload"]["porla_http_client"] = Packages::HttpClient::Load;
        package["preload"]["porla_http_server"] = Packages::HttpServer::Load;
        package["preload"]["porla_presets"]     = Packages::Presets::Load;
        package["preload"]["porla_runtime"]     = Packages::Runtime::Load;
        package["preload"]["porla_sessions"]    = Packages::Sessions::Load;
        package["preload"]["porla_sockets"]     = Packages::Sockets::Load;
        package["preload"]["porla_timers"]      = Packages::Timers::Load;

        auto searcher = [this](sol::this_state ts, std::string name) -> sol::variadic_results
        {
            sol::state_view lua_view(ts);
            sol::variadic_results results;

            std::string base = name;
            std::replace(base.begin(), base.end(), '.', '/');

            const std::string candidates[] = { base + ".lua", base + "/init.lua" };

            std::string tried;

            for (const auto& candidate : candidates)
            {
                const auto it = source.sources.find(candidate);

                if (it == source.sources.end())
                {
                    tried += "\n\tno source '" + candidate + "' in plugin";
                    continue;
                }

                sol::load_result chunk = lua_view.load_buffer(
                    it->second.data(),
                    it->second.size(),
                    "@" + candidate);

                if (!chunk.valid())
                {
                    sol::error err = chunk;
                    throw std::runtime_error(
                        "error loading module '" + name + "' from plugin source '"
                        + candidate + "':\n\t" + err.what());
                }

                results.push_back({ ts, sol::in_place, chunk.get<sol::protected_function>() });
                results.push_back({ ts, sol::in_place, candidate });

                return results;
            }

            results.push_back({ ts, sol::in_place, tried });

            return results;
        };

        sol::table searchers = package["searchers"].valid()
            ? package["searchers"].get<sol::table>()
            : package["loaders"].get<sol::table>();

        sol::protected_function table_insert = lua["table"]["insert"];
        table_insert(searchers, 2, sol::make_object(lua, searcher));

        lua_state            = std::make_shared<LuaState>(load_options.io, load_options.sessions, lua);
        lua_state->app       = load_options.http_server;
        lua_state->curl      = load_options.curl_multi;
        lua_state->db        = load_options.db;
        lua_state->lua       = lua;
        lua_state->plugin_id = load_options.plugin_id;

        lua.registry()["state"] = std::weak_ptr(lua_state);

        lua.globals()["print"] = &Print;
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
        BOOST_LOG_TRIVIAL(error) << "plugin[" << opts.plugin_id << "] Failed to load plugin source from " << path;
        return nullptr;
    }

    if (source->sources.find(source->entrypoint) == source->sources.end())
    {
        BOOST_LOG_TRIVIAL(error) << "plugin[" << opts.plugin_id << "] Plugin entry point (plugin.lua) not found for " << path;
        return nullptr;
    }

    try
    {
        auto state = std::make_unique<State>(opts);
        state->source = *source;

        sol::load_result chunk = state->lua.load_buffer(
            state->source.sources.at(state->source.entrypoint).data(),
            state->source.sources.at(state->source.entrypoint).size(),
            "@" + state->source.entrypoint);

        if (!chunk.valid())
        {
            sol::error err = chunk;
            BOOST_LOG_TRIVIAL(error) << "plugin[" << opts.plugin_id << "] Failed to load plugin: " << err.what();
            return nullptr;
        }
        sol::protected_function_result result = chunk.get<sol::protected_function>()();

        if (!result.valid())
        {
            BOOST_LOG_TRIVIAL(error)
                << "plugin[" << opts.plugin_id << "] Failed to run plugin: " << DescribeError(result);
            return nullptr;
        }

        if (result.return_count() < 1 || result.get_type() != sol::type::table)
        {
            BOOST_LOG_TRIVIAL(error)
                << "plugin[" << opts.plugin_id << "] Plugin did not return a table (got "
                << sol::type_name(state->lua.lua_state(), result.get_type()) << ")";
            return nullptr;
        }

        state->tbl = result.get<sol::table>();

        sol::optional<sol::protected_function> init = state->tbl["init"];

        if (init && init->valid())
        {
            sol::object cfg = sol::lua_nil;

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
                        BOOST_LOG_TRIVIAL(error) << "plugin[" << opts.plugin_id << "] Failed to evaluate plugin config: " << DescribeError(config_value);
                        return nullptr;
                    }
                }
                else
                {
                    sol::error err = config_result;
                    BOOST_LOG_TRIVIAL(error) << "plugin[" << opts.plugin_id << "] Failed to parse plugin config: " << err.what();
                    return nullptr;
                }
            }

            sol::protected_function_result init_result = (*init)(cfg);

            if (!init_result.valid())
            {
                BOOST_LOG_TRIVIAL(error) << "plugin[" << opts.plugin_id << "] Failed to run plugin initializer: " << DescribeError(init_result);
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

    try
    {
        sol::optional<sol::protected_function> destroy = m_state->tbl["destroy"];

        if (destroy && destroy->valid())
        {
            sol::protected_function_result result = (*destroy)();

            if (!result.valid())
            {
                BOOST_LOG_TRIVIAL(error)
                    << "plugin[" << m_state->load_options.plugin_id << "] Error in destroy: " << DescribeError(result);
            }
        }
    }
    catch (const std::exception& err)
    {
        BOOST_LOG_TRIVIAL(error) << "Exception during plugin teardown: " << err.what();
    }
    catch (...)
    {
        BOOST_LOG_TRIVIAL(error) << "Unknown exception during plugin teardown";
    }

    m_state->lua_state.reset();
    m_state.reset();
}

porla::Lua::PluginSource& Plugin::Source() const
{
    return m_state->source;
}
