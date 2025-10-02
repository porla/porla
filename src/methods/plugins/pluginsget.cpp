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
    auto plugin = m_plugin_engine.Plugins().find(req.id);

    if (plugin == m_plugin_engine.Plugins().end())
    {
        return cb.Error(-1, "Plugin not found");
    }

    return cb.Ok(PluginsGetRes{
        .config = "",
        .path   = ""
    });
}
