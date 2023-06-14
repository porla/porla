#include "pluginengine.hpp"

#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>
#include <utility>

#include "plugin.hpp"

using porla::Lua::Plugins::Plugin;
using porla::Lua::Plugins::PluginEngine;
using porla::Lua::Plugins::PluginEngineOptions;

PluginEngine::PluginEngine(const PluginEngineOptions& options)
{
    for (const auto& dir : options.plugins)
    {
        BOOST_LOG_TRIVIAL(info) << "Loading plugin " << dir;

        const PluginLoadOptions plugin_load_options{
            .config  = options.config,
            .dir     = dir,
            .io      = options.io,
            .session = options.session
        };

        auto plugin = Plugin::Load(plugin_load_options);

        if (plugin == nullptr)
        {
            continue;
        }

        PluginState plugin_state{
            .plugin = std::move(plugin)
        };

        m_plugins.emplace_back(std::move(plugin_state));
    }
}

PluginEngine::~PluginEngine() = default;

void PluginEngine::UnloadAll()
{
    m_plugins.clear();
}
