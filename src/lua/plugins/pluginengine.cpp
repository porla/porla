#include "pluginengine.hpp"

#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>
#include <utility>

#include "plugin.hpp"

#include "../../data/statement.hpp"

using porla::Data::Statement;
using porla::Lua::Plugins::Plugin;
using porla::Lua::Plugins::PluginEngine;
using porla::Lua::Plugins::PluginEngineOptions;
using porla::Lua::Plugins::PluginInstallOptions;

PluginEngine::PluginEngine(const PluginEngineOptions& options)
    : m_options(options)
{
}

PluginEngine::~PluginEngine() = default;

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
        .path   = options.path,
        .plugin = nullptr
    };

    m_plugins.insert({ plugin_name, std::move(plugin_state) });

    if (options.enable)
    {
        BOOST_LOG_TRIVIAL(info) << "Enabling plugin";

        auto state = m_plugins.find(plugin_name);

        if (state != m_plugins.end())
        {
            BOOST_LOG_TRIVIAL(info) << "Doing the cha cha cha";

            const PluginLoadOptions plugin_load_options{
                .config  = m_options.config,
                .dir     = options.path,
                .io      = m_options.io,
                .session = m_options.session
            };

            state->second.plugin = Plugin::Load(plugin_load_options);
        }
    }
}

void PluginEngine::Uninstall(const std::string& name, std::error_code& ec)
{
    BOOST_LOG_TRIVIAL(info) << "Uninstalling plugin " << name;

    auto state = m_plugins.find(name);

    if (state == m_plugins.end())
    {
        return;
    }

    auto stmt = Statement::Prepare(m_options.db, "DELETE FROM plugins WHERE name = $1");
    stmt.Bind(1, std::string_view(name));
    stmt.Execute();

    if (state->second.plugin != nullptr)
    {
        state->second.plugin.reset();
    }
}

void PluginEngine::UnloadAll()
{
    m_plugins.clear();
}