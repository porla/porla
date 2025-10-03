#include "pluginsreload.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;

using porla::Methods::PluginsReload;
using porla::Methods::PluginsReloadReq;
using porla::Methods::PluginsReloadRes;

PluginsReload::PluginsReload(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsReload::Invoke(const PluginsReloadReq& req, WriteCb<PluginsReloadRes> cb)
{
    m_plugin_engine.Reload(req.id);
    cb.Ok({});
}
