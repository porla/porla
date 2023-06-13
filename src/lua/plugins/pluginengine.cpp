#include "pluginengine.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/torrent_info.hpp>
#include <sol/sol.hpp>
#include <utility>
#include <sqlite3.h>

#include "../usertypes/cron.hpp"
#include "../usertypes/fsdirectory.hpp"
#include "../usertypes/fsdirectoryentry.hpp"
#include "../usertypes/session.hpp"
#include "../usertypes/sqlite3db.hpp"
#include "../usertypes/sqlite3statement.hpp"

#include "../../session.hpp"

#define PORLA_OPTS_KEY "__c_porla_opts"

namespace fs = std::filesystem;

using porla::Lua::Plugins::PluginEngine;
using porla::Lua::Plugins::PluginEngineOptions;

static sol::object TomlNode2LuaObject(sol::state& lua, const toml::node& node)
{
    switch (node.type())
    {
        case toml::node_type::none:
            return {};

        case toml::node_type::table:
        {
            sol::table tbl = lua.create_table();

            for (const auto& [ key, value ] : *node.as_table())
            {
                tbl[key.str()] = TomlNode2LuaObject(lua, value);
            }

            return tbl;
        }

        case toml::node_type::array:
        {
            sol::table tbl = lua.create_table();
            const auto& arr = *node.as_array();

            for (int i = 0; i < arr.size(); i++)
            {
                tbl[i] = TomlNode2LuaObject(lua, arr[i]);
            }

            return tbl;
        }

        case toml::node_type::string:
            return sol::make_object(lua, node.as_string()->get());

        case toml::node_type::integer:
            return sol::make_object(lua, node.as_integer()->get());

        case toml::node_type::floating_point:
            return sol::make_object(lua, node.as_floating_point()->get());

        case toml::node_type::boolean:
            return sol::make_object(lua, node.as_boolean()->get());

        case toml::node_type::date:
            break;
        case toml::node_type::time:
            break;
        case toml::node_type::date_time:
            break;
    }

    return {};
}

struct PluginOptions
{
    toml::table              config;
    boost::asio::io_context& io;
    porla::ISession&         session;
};

class Plugin
{
public:
    virtual ~Plugin() = default;
    virtual bool Init() = 0;

protected:
    static sol::state DefaultLua(const PluginOptions& opts)
    {
        sol::state lua;
        lua.globals()[PORLA_OPTS_KEY] = opts;

        lua.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::string);

        porla::Lua::UserTypes::Cron::Register(lua);
        porla::Lua::UserTypes::FsDirectory::Register(lua);
        porla::Lua::UserTypes::FsDirectoryEntry::Register(lua);
        porla::Lua::UserTypes::Session::Register(lua);
        porla::Lua::UserTypes::Sqlite3Db::Register(lua);
        porla::Lua::UserTypes::Sqlite3Statement::Register(lua);

        return std::move(lua);
    }
};

class DirectoryBasedPlugin : public Plugin
{
public:
    DirectoryBasedPlugin(const PluginOptions& options, fs::path path)
        : m_lua(DefaultLua(options))
        , m_path(std::move(path))
        , m_options(options)
    {
    }

    bool Init() override
    {
        const auto plugin_lua = m_path / "plugin.lua";

        if (!fs::is_regular_file(plugin_lua))
        {
            BOOST_LOG_TRIVIAL(warning) << "Expected file " << plugin_lua << " not found";
            return false;
        }

        // Put the plugin directory in the package path
        const std::string package_path = m_lua["package"]["path"];
        m_lua["package"]["path"] = package_path + (!package_path.empty() ? ";" : "") + m_path.string() + "/?.lua";

        m_lua["package"]["preload"]["config"] = [&](sol::this_state s)
        {
            return TomlNode2LuaObject(m_lua, m_options.config);
        };

        m_lua["package"]["preload"]["cron"] = [&](sol::this_state s)
        {
            sol::state_view lua{s};
            sol::table cron = lua.create_table();

            cron["schedule"] = [&](const sol::table& args)
            {
                return std::make_shared<porla::Lua::UserTypes::Cron>(m_options.io, args);
            };

            return cron;
        };

        m_lua["package"]["preload"]["fs"] = [](sol::this_state s)
        {
            sol::state_view lua{s};
            sol::table fs = lua.create_table();

            fs["abs"] = [](const std::string& path)
            {
                return fs::absolute(path).string();
            };

            fs["dir"] = [](const std::string& path)
            {
                std::vector<std::string> paths;
                for (const auto& e : fs::directory_iterator(path)) paths.emplace_back(e.path());
                return paths;
            };

            fs["exists"] = [](const std::string& path)
            {
                return fs::exists(path);
            };

            fs["ext"] = [](const std::string& path)
            {
                return fs::path(path).extension().string();
            };

            return fs;
        };

        m_lua["package"]["preload"]["log"] = [](sol::this_state s)
        {
            sol::state_view lua{s};
            sol::table log = lua.create_table();

            log["error"]   = [](const std::string& message) { BOOST_LOG_TRIVIAL(error) << message; };
            log["info"]    = [](const std::string& message) { BOOST_LOG_TRIVIAL(info) << message; };
            log["warning"] = [](const std::string& message) { BOOST_LOG_TRIVIAL(warning) << message; };

            return log;
        };

        m_lua["package"]["preload"]["sqlite"] = [](sol::this_state s)
        {
            sol::state_view lua{s};
            sol::table sql = lua.create_table();
            sql["open"] = [&](const sol::object& args)
            {
                sqlite3* db;
                sqlite3_open(args.as<std::string>().c_str(), &db);
                return std::make_shared<porla::Lua::UserTypes::Sqlite3Db>(db);
            };
            return sql;
        };

        m_lua["package"]["preload"]["torrents"] = [](sol::this_state s)
        {
            sol::state_view lua{s};
            sol::table torrents = lua.create_table();

            torrents["load"] = [](const std::string& path)
            {
                return std::make_shared<lt::torrent_info>(path);
            };

            torrents["has"] = [](sol::this_state s, const sol::object& arg)
            {
                sol::state_view lua{s};
                const auto options = lua.globals()[PORLA_OPTS_KEY].get<const PluginOptions&>();

                if (arg.is<std::shared_ptr<lt::torrent_info>>())
                {
                    const auto ti = arg.as<std::shared_ptr<lt::torrent_info>>();
                    return options.session.Torrents().find(ti->info_hashes()) != options.session.Torrents().end();
                }

                return false;
            };

            return torrents;
        };

        try
        {
            m_plugin = m_lua.create_table();

            sol::environment env(m_lua, sol::create, m_lua.globals());
            env["porla"] = m_plugin;

            m_lua.script_file(plugin_lua.string(), env);

            if (m_plugin["init"].is<sol::function>())
            {
                m_plugin["init"]();
            }

            return true;
        }
        catch (const sol::error& err)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to load plugin: " << err.what();
        }

        return false;
    }

private:
    sol::state m_lua;
    PluginOptions m_options;
    fs::path m_path;
    sol::table m_plugin;
};

class PluginEngine::State
{
public:
    explicit State(const PluginEngineOptions& options)
        : m_opts(options)
    {
        for (const auto& dir : m_opts.plugins)
        {
            BOOST_LOG_TRIVIAL(info) << "Loading plugin " << dir;

            const PluginOptions plugin_options{
                .config  = options.config,
                .io      = options.io,
                .session = options.session
            };

            auto plugin = std::make_unique<DirectoryBasedPlugin>(plugin_options, dir);

            if (!plugin->Init())
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to load plugin: " << dir;
                continue;
            }

            m_plugins.emplace_back(std::move(plugin));
        }
    }

private:
    const PluginEngineOptions& m_opts;
    std::vector<std::unique_ptr<Plugin>> m_plugins;
};

PluginEngine::PluginEngine(const PluginEngineOptions& options)
    : m_state(std::make_unique<State>(options))
{
}

PluginEngine::~PluginEngine() = default;
