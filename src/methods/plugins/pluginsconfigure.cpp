#include "pluginsconfigure.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;

using porla::Methods::PluginsConfigure;
using porla::Methods::PluginsConfigureReq;
using porla::Methods::PluginsConfigureRes;

PluginsConfigure::PluginsConfigure(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsConfigure::Invoke(const PluginsConfigureReq& req, WriteCb<PluginsConfigureRes> cb)
{
    m_plugin_engine.Configure(req.id, req.config);
    cb.Ok({});
}
