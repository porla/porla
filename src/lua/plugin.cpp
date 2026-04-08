#include "plugin.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>

#include "packages.hpp"
#include "registry.hpp"
#include "types.hpp"

#include "../config.hpp"
#include "../zip.hpp"

namespace fs = std::filesystem;
using porla::Lua::Plugin;
using porla::Lua::PluginLoadOptions;

struct Plugin::State
{
    std::map<std::string, std::vector<char>> files;
    PluginLoadOptions                        load_options;
    sol::state                               lua;
    std::optional<Plugin::Meta>              meta;
};

static sol::state CreateLuaState(const PluginLoadOptions& opts)
{
    sol::state lua;

    lua.open_libraries(
        sol::lib::base,
        sol::lib::io,
        sol::lib::os,
        sol::lib::package,
        sol::lib::string,
        sol::lib::table);

    lua.globals()["__load_opts"] = opts;
    lua.globals()["porla"]       = lua.create_table();
    lua.registry()["db"]         = porla::Lua::Registry::Sqlite3{.db = opts.config.db};

    porla::Lua::Packages::Config::Register(lua);
    porla::Lua::Packages::Cron::Register(lua);
    porla::Lua::Packages::Events::Register(lua);
    porla::Lua::Packages::FileSystem::Register(lua);
    porla::Lua::Packages::HttpClient::Register(lua);
    porla::Lua::Packages::Json::Register(lua);
    porla::Lua::Packages::Log::Register(lua);
    porla::Lua::Packages::PQL::Register(lua);
    porla::Lua::Packages::Presets::Register(lua);
    porla::Lua::Packages::Process::Register(lua);
    porla::Lua::Packages::Sessions::Register(lua);
    porla::Lua::Packages::Sqlite::Register(lua);
    porla::Lua::Packages::Timers::Register(lua);
    porla::Lua::Packages::Torrents::Register(lua);
    porla::Lua::Packages::Workflows::Register(lua);

    porla::Lua::Types::LtAddTorrentParams::Register(lua);
    porla::Lua::Types::LtAnnounceEndpoint::Register(lua);
    porla::Lua::Types::LtAnnounceEntry::Register(lua);
    porla::Lua::Types::LtAnnounceInfohash::Register(lua);
    porla::Lua::Types::LtDownloadPriority::Register(lua);
    porla::Lua::Types::LtInfoHash::Register(lua);
    porla::Lua::Types::LtPeerInfo::Register(lua);
    porla::Lua::Types::LtSettingsPack::Register(lua);
    porla::Lua::Types::LtStorageMode::Register(lua);
    porla::Lua::Types::LtTorrentFlags::Register(lua);
    porla::Lua::Types::LtTorrentHandle::Register(lua);
    porla::Lua::Types::LtTorrentInfo::Register(lua);
    porla::Lua::Types::LtTorrentStatus::Register(lua);

    return lua;
}

std::unique_ptr<Plugin> Plugin::LoadFromArchive(
    const std::vector<char>& buffer,
    const std::optional<std::string>& config,
    const PluginLoadOptions& opts)
{
    auto state = std::make_unique<State>(State{
        .files        = Zip::Load(buffer),
        .load_options = opts,
        .lua          = CreateLuaState(opts),
        .meta         = std::nullopt
    });

    try
    {
        state->lua.script(
            std::string(
                state->files.at("plugin.lua").begin(),
                state->files.at("plugin.lua").end()));

        if (state->lua.globals()["porla"]["init"].is<sol::function>())
        {
            state->lua.globals()["porla"]["init"](
                config.has_value()
                    ? sol::object(state->lua.script(config.value()))
                    : sol::nil);
        }

        return std::unique_ptr<Plugin>(new Plugin(std::move(state)));
    }
    catch (const sol::error& err)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load plugin: " << err.what();
    }

    return nullptr;
}

std::unique_ptr<Plugin> Plugin::LoadFromPath(
    const std::filesystem::path& path,
    const std::optional<std::string>& config,
    const PluginLoadOptions& opts)
{
    auto state = std::make_unique<State>(State{
        .files        = {},
        .load_options = opts,
        .lua          = CreateLuaState(opts),
        .meta         = std::nullopt
    });

    try
    {
        const fs::path plugin_lua = path / "plugin.lua";
        state->lua.script_file(plugin_lua.string());

        std::optional<sol::table> plugin_meta;

        if (state->lua["plugin"].is<sol::table>())
        {
            plugin_meta = state->lua["plugin"];
        }

        if (plugin_meta)
        {
            state->meta = Plugin::Meta{
                .name    = std::nullopt,
                .version = std::nullopt
            };

            if (plugin_meta.value()["name"].is<std::string>())
                state->meta->name = plugin_meta.value()["name"];

            if (plugin_meta.value()["version"].is<std::string>())
                state->meta->version = plugin_meta.value()["version"];
        }

        if (state->lua.globals()["porla"]["init"].is<sol::function>())
        {
            state->lua.globals()["porla"]["init"](
                config.has_value()
                    ? sol::object(state->lua.script(config.value()))
                    : sol::nil);
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

std::optional<Plugin::Meta> Plugin::GetMeta()
{
    return m_state->meta;
}
