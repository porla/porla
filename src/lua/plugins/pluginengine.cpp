#include "pluginengine.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/torrent_info.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>

#include "../usertypes/cron.hpp"
#include "../usertypes/fsdirectory.hpp"
#include "../usertypes/fsdirectoryentry.hpp"
#include "../usertypes/session.hpp"
#include "../usertypes/sqlite3db.hpp"
#include "../usertypes/sqlite3statement.hpp"

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

class PluginEngine::State
{
public:
    explicit State(const PluginEngineOptions& options)
        : m_opts(options)
    {
        m_lua.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::string);

        UserTypes::Cron::Register(m_lua);
        UserTypes::FsDirectory::Register(m_lua);
        UserTypes::FsDirectoryEntry::Register(m_lua);
        UserTypes::Session::Register(m_lua);
        UserTypes::Sqlite3Db::Register(m_lua);
        UserTypes::Sqlite3Statement::Register(m_lua);

        sol::table fs = m_lua.create_table();
        fs["Directory"] = [&](const sol::object& args)
        {
            return std::make_unique<UserTypes::FsDirectory>(args);
        };

        sol::table sql = m_lua.create_table();
        sql["open"] = [&](const sol::object& args)
        {
            sqlite3* db;
            sqlite3_open(args.as<std::string>().c_str(), &db);
            return std::make_shared<UserTypes::Sqlite3Db>(db);
        };

        m_lua["cron"] = [&](const sol::table& args)
        {
            return std::make_shared<UserTypes::Cron>(m_opts.io, args);
        };

        m_lua["fs"] = fs;
        m_lua["load_torrent_file"] = [&](const std::string& path)
        {
            return std::make_shared<lt::torrent_info>(path);
        };
        m_lua["sqlite3"] = sql;

        sol::table ctx = m_lua.create_table();
        ctx["config"] = TomlNode2LuaObject(m_lua, m_opts.config);
        ctx["session"] = std::make_shared<UserTypes::Session>(m_opts.session);

        if (!options.plugins_dir.empty())
        {
            for (const auto& file : fs::directory_iterator(options.plugins_dir))
            {
                if (file.path().extension() != ".lua") continue;

                try
                {
                    sol::table plugin = m_lua.script_file(file.path());
                    plugin["init"](ctx);

                    m_plugins.emplace_back(plugin);
                }
                catch (const sol::error& err)
                {
                    BOOST_LOG_TRIVIAL(error) << "Failed to load plugin: " << err.what();
                }
            }
        }
    }

private:
    const PluginEngineOptions& m_opts;
    sol::state m_lua;
    std::vector<sol::object> m_plugins;
};

PluginEngine::PluginEngine(const PluginEngineOptions& options)
    : m_state(std::make_unique<State>(options))
{
}

PluginEngine::~PluginEngine() = default;
