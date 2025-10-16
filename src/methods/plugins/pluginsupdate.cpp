#include "pluginsupdate.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;

using porla::Methods::PluginsUpdate;
using porla::Methods::PluginsUpdateReq;
using porla::Methods::PluginsUpdateRes;

PluginsUpdate::PluginsUpdate(porla::Lua::PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsUpdate::Invoke(const PluginsUpdateReq& req, WriteCb<PluginsUpdateRes> cb)
{
    auto plugin = m_plugin_engine.Plugins().find(req.id);

    if (plugin == m_plugin_engine.Plugins().end())
    {
        return cb.Error(-1, "Plugin not found");
    }

    if (plugin->second.type == "archive")
    {
        return cb.Ok(PluginsUpdateRes{});
    }

    if (plugin->second.type == "path")
    {
        return cb.Ok(PluginsUpdateRes{});
    }

    return cb.Error(-2, "Invalid plugin type");
}
