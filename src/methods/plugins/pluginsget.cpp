#include "pluginsget.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;

using porla::Methods::PluginsGet;
using porla::Methods::PluginsGetReq;
using porla::Methods::PluginsGetRes;

PluginsGet::PluginsGet(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsGet::Invoke(const PluginsGetReq& req, WriteCb<PluginsGetRes> cb)
{
    const auto plugin_state = m_plugin_engine.Plugins().find(req.id);

    if (plugin_state == m_plugin_engine.Plugins().end())
    {
        return cb.Error(-1, "Plugin not found");
    }

    const auto manifest = plugin_state->second.plugin->GetManifest();

    return cb.Ok(PluginsGetRes{
        .id       = req.id,
        .type     = "",
        .name     = manifest.has_value() ? manifest->name    : std::nullopt,
        .version  = manifest.has_value() ? manifest->version : std::nullopt,
        .config   = plugin_state->second.config,
        .metadata = plugin_state->second.metadata
    });
}
