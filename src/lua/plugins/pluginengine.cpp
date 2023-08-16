#include "pluginengine.hpp"

#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>
#include <utility>

#include "plugin.hpp"

#include "../../config.hpp"
#include "../../data/statement.hpp"

using porla::Data::Statement;
using porla::Lua::Plugins::Plugin;
using porla::Lua::Plugins::PluginEngine;
using porla::Lua::Plugins::PluginEngineOptions;
using porla::Lua::Plugins::PluginInstallOptions;
using porla::Lua::Plugins::PluginState;

PluginEngine::PluginEngine(PluginEngineOptions options)
    : m_options(options)
{
    auto stmt = Statement::Prepare(m_options.db, "SELECT enabled,name,path,config FROM plugins");
    stmt.Step(
        [&](const auto& row)
        {
            const PluginLoadOptions plugin_load_options{
                .config        = m_options.config,
                .io            = m_options.io,
                .path          = row.GetStdString(2),
                .plugin_config = row.GetStdString(3),
                .sessions      = m_options.sessions
            };

            PluginState plugin_state{
                .can_configure = true,
                .can_uninstall = true,
                .config        = plugin_load_options.plugin_config,
                .path          = plugin_load_options.path,
                .plugin        = row.GetInt32(0) > 0
                    ? Plugin::Load(plugin_load_options)
                    : nullptr
            };

            m_plugins.insert({ row.GetStdString(1), std::move(plugin_state) });

            return SQLITE_OK;
        });

    const auto& workflow_dir = m_options.config.workflow_dir;

    if (workflow_dir.has_value() && fs::exists(workflow_dir.value()))
    {
        BOOST_LOG_TRIVIAL(debug) << "Loading all workflow files in " << workflow_dir.value();

        for (const auto& file: fs::directory_iterator(workflow_dir.value()))
        {
            if (file.path().extension() != ".lua")
            {
                BOOST_LOG_TRIVIAL(debug) << "Skipping " << file.path() << ", wrong file extension";
                continue;
            }

            BOOST_LOG_TRIVIAL(info) << "Loading workflow " << file.path().filename();

            const PluginLoadOptions plugin_load_options{
                .config        = m_options.config,
                .io            = m_options.io,
                .path          = file,
                .plugin_config = std::nullopt,
                .sessions      = m_options.sessions
            };

            PluginState plugin_state{
                .can_configure = false,
                .can_uninstall = false,
                .config        = plugin_load_options.plugin_config,
                .path          = plugin_load_options.path,
                .plugin        = Plugin::Load(plugin_load_options)
            };

            m_plugins.insert({ file.path().filename(), std::move(plugin_state) });
        }
    }
}

PluginEngine::~PluginEngine() = default;

void PluginEngine::Configure(const std::string& name, const std::optional<std::string>& config)
{
    auto stmt = Statement::Prepare(m_options.db, "UPDATE plugins SET config = $1 where name = $2");
    stmt.Bind(1, config);
    stmt.Bind(2, std::string_view(name));
    stmt.Execute();

    auto state = m_plugins.find(name);

    if (state == m_plugins.end())
    {
        return;
    }

    if (state->second.plugin != nullptr)
    {
        state->second.plugin.reset();
        state->second.plugin = nullptr;
    }

    const PluginLoadOptions plugin_load_options{
        .config        = m_options.config,
        .io            = m_options.io,
        .path          = state->second.path,
        .plugin_config = config,
        .sessions      = m_options.sessions
    };

    state->second.config = config;
    state->second.plugin = Plugin::Load(plugin_load_options);
}

void PluginEngine::Install(const PluginInstallOptions& options, std::error_code& ec)
{
    // TODO: Check if the path is in the database
    // TODO: Check if any running plugin has this path

    const auto plugin_name = options.path.stem().string();

    BOOST_LOG_TRIVIAL(info) << "Installing plugin " << plugin_name;

    const auto path_str = options.path.string();

    auto stmt = Statement::Prepare(m_options.db, "INSERT INTO plugins (name, path, config, enabled)"
                                                 "VALUES ($1, $2, $3, $4);");
    stmt.Bind(1, std::string_view(plugin_name));
    stmt.Bind(2, std::string_view(path_str));
    stmt.Bind(3, options.config);
    stmt.Bind(4, true);
    stmt.Execute();

    // Add it to our view of plugins, but don't enable it.
    PluginState plugin_state{
        .can_configure = true,
        .can_uninstall = true,
        .config        = options.config,
        .path          = options.path,
        .plugin        = nullptr
    };

    m_plugins.insert({ plugin_name, std::move(plugin_state) });

    if (options.enable)
    {
        BOOST_LOG_TRIVIAL(info) << "Enabling plugin";

        auto state = m_plugins.find(plugin_name);

        if (state != m_plugins.end())
        {
            const PluginLoadOptions plugin_load_options{
                .config        = m_options.config,
                .io            = m_options.io,
                .path          = options.path,
                .plugin_config = options.config,
                .sessions      = m_options.sessions
            };

            state->second.plugin = Plugin::Load(plugin_load_options);
        }
    }
}

std::map<std::string, PluginState> &PluginEngine::Plugins()
{
    return m_plugins;
}

void PluginEngine::Reload(const std::string& name)
{
    auto state = m_plugins.find(name);

    if (state == m_plugins.end())
    {
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Reloading plugin " << name;

    if (state->second.plugin != nullptr)
    {
        state->second.plugin.reset();
        state->second.plugin = nullptr;
    }

    const PluginLoadOptions plugin_load_options{
        .config        = m_options.config,
        .io            = m_options.io,
        .path          = state->second.path,
        .plugin_config = state->second.config,
        .sessions      = m_options.sessions
    };

    state->second.plugin = Plugin::Load(plugin_load_options);
}

void PluginEngine::Uninstall(const std::string& name, std::error_code& ec)
{
    auto state = m_plugins.find(name);

    if (state == m_plugins.end())
    {
        BOOST_LOG_TRIVIAL(warning) << "Plugin not found: " << name;
        return;
    }

    if (!state->second.can_uninstall)
    {
        BOOST_LOG_TRIVIAL(warning) << "Cannot uninstall plugin " << name;
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Uninstalling plugin " << name;

    auto stmt = Statement::Prepare(m_options.db, "DELETE FROM plugins WHERE name = $1");
    stmt.Bind(1, std::string_view(name));
    stmt.Execute();

    if (state->second.plugin != nullptr)
    {
        state->second.plugin.reset();
        state->second.plugin = nullptr;
    }

    m_plugins.erase(state);
}

void PluginEngine::UnloadAll()
{
    m_plugins.clear();
}
