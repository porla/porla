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

    const auto meta = plugin_state->second.plugin->GetMeta();

    return cb.Ok(PluginsGetRes{
        .plugin = PluginsGetRes::Plugin{
            .id       = req.id,
            .type     = plugin_state->second.type,
            .name     = meta.has_value() ? meta->name    : std::nullopt,
            .version  = meta.has_value() ? meta->version : std::nullopt,
            .config   = plugin_state->second.config,
            .metadata = plugin_state->second.metadata.has_value()
                ? plugin_state->second.metadata.value()
                : std::map<std::string, nlohmann::json>()
        }
    });
}
