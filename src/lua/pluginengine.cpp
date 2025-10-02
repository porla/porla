#include "pluginengine.hpp"

#include <utility>

#include <boost/algorithm/string/replace.hpp>
#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>
#include <toml++/toml.hpp>
#include <zip.h>

#include "plugin.hpp"

#include "../config.hpp"
#include "../data/statement.hpp"

using porla::Data::Statement;
using porla::Lua::Plugin;
using porla::Lua::PluginEngine;
using porla::Lua::PluginEngineOptions;
using porla::Lua::PluginState;

PluginEngine::PluginEngine(PluginEngineOptions options)
    : m_options(options)
{
}

PluginEngine::~PluginEngine() = default;

void PluginEngine::Configure(int id, const std::optional<std::string>& config)
{
    if (m_plugins.find(id) == m_plugins.end())
    {
        return;
    }

    auto update_stmt = Statement::Prepare(
        m_options.db,
        "UPDATE plugins SET config = $1 WHERE id = $2");
    update_stmt.Bind(1, config);
    update_stmt.Bind(2, id);
    update_stmt.Execute();

    Reload(id);
}

int PluginEngine::InstallFromPath(const fs::path& path, std::optional<std::string> config, const nlohmann::json& metadata)
{
    auto install_stmt = Statement::Prepare(
        m_options.db,
        "INSERT INTO plugins (type, data, config, metadata) VALUES ('path', $1, $2, $3)");
    install_stmt.Bind(1, std::string_view(path.string()));
    install_stmt.Bind(2, config);
    install_stmt.Bind(3, metadata.is_null()
        ? std::optional<std::string_view>()
        : std::string_view(metadata.dump()));

    install_stmt.Execute();

    int plugin_id = sqlite3_last_insert_rowid(m_options.db);

    BOOST_LOG_TRIVIAL(info) << "plugin[" << plugin_id << "] installed with path " << path;

    Load(plugin_id);

    return plugin_id;
}

int PluginEngine::InstallFromArchive(const std::vector<char>& buffer, std::optional<std::string> config, const nlohmann::json& metadata)
{
    auto install_stmt = Statement::Prepare(
        m_options.db,
        "INSERT INTO plugins (type, data, config, metadata) VALUES ('archive', $1, $2, $3)");
    install_stmt.Bind(1, buffer);
    install_stmt.Bind(2, config);
    install_stmt.Bind(3, metadata.is_null()
        ? std::optional<std::string_view>()
        : std::string_view(metadata.dump()));

    install_stmt.Execute();

    int plugin_id = sqlite3_last_insert_rowid(m_options.db);

    BOOST_LOG_TRIVIAL(info) << "plugin[" << plugin_id << "] installed with archive (" << buffer.size() << " bytes)";

    Load(plugin_id);

    return plugin_id;
}

void PluginEngine::LoadAll()
{
    std::set<int> plugin_ids;

    auto load_stmt = Statement::Prepare(
        m_options.db,
        "SELECT id FROM plugins ORDER BY id ASC");

    load_stmt.Step([&plugin_ids](const auto& row)
    {
        plugin_ids.insert(row.GetInt32(0));
        return SQLITE_OK;
    });

    for (const auto plugin_id : plugin_ids)
    {
        Load(plugin_id);
    }
}

void PluginEngine::Load(int id)
{
    if (m_plugins.find(id) != m_plugins.end())
    {
        BOOST_LOG_TRIVIAL(error) << "Plugin already loaded";
        return;
    }

    auto load_stmt = Statement::Prepare(
        m_options.db,
        "SELECT id,type,data,config FROM plugins WHERE id = $1");
    load_stmt.Bind(1, id);
    load_stmt.Step(
        [this, id](const auto& row)
        {
            const auto& load_options = PluginLoadOptions{
                .config        = m_options.config,
                .io            = m_options.io,
                .sessions      = m_options.sessions
            };

            std::unique_ptr<Plugin> plugin = nullptr;

            const auto type = row.GetStdString(1);
            const auto conf = row.GetOptionalStdString(3);

            if (type == "path")
            {
                BOOST_LOG_TRIVIAL(info) << "plugin[" << id << "] Loading from path";
                plugin = Plugin::LoadFromPath(row.GetStdString(2), conf, load_options);
            }
            else if (type == "archive")
            {
                BOOST_LOG_TRIVIAL(info) << "plugin[" << id << "] Loading from archive";
                plugin = Plugin::LoadFromArchive(row.GetBuffer(2), conf, load_options);
            }
            else
            {
                BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Invalid type: " << type;
                return SQLITE_OK;
            }

            if (plugin == nullptr)
            {
                BOOST_LOG_TRIVIAL(warning) << "plugin[" << id << "] Failed to load";
                return SQLITE_OK;
            }

            m_plugins.emplace(id, PluginState{
                .plugin   = std::move(plugin)
            });

            return SQLITE_OK;
        });
}

std::map<int, PluginState> &PluginEngine::Plugins()
{
    return m_plugins;
}

void PluginEngine::Reload(int id)
{
    if (m_plugins.find(id) != m_plugins.end())
    {
        Unload(id);
    }

    Load(id);
}

void PluginEngine::Uninstall(int id)
{
    auto uninstall_stmt = Statement::Prepare(
        m_options.db,
        "DELETE FROM plugins WHERE id = $1");
    uninstall_stmt.Bind(1, id);
    uninstall_stmt.Execute();

    if (m_plugins.find(id) != m_plugins.end())
    {
        Unload(id);
    }
}

void PluginEngine::Unload(int id)
{
    if (m_plugins.find(id) == m_plugins.end())
    {
        BOOST_LOG_TRIVIAL(error) << "Plugin not loaded";
        return;
    }

    m_plugins.erase(id);
}

void PluginEngine::UnloadAll()
{
    m_plugins.clear();
}
