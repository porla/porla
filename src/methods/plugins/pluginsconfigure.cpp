#include "pluginsconfigure.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugins/plugin.hpp"
#include "../../lua/plugins/pluginengine.hpp"

using porla::Lua::Plugins::PluginEngine;
using porla::Lua::Plugins::PluginInstallOptions;

using porla::Methods::PluginsConfigure;
using porla::Methods::PluginsConfigureReq;
using porla::Methods::PluginsConfigureRes;

PluginsConfigure::PluginsConfigure(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsConfigure::Invoke(const PluginsConfigureReq& req, WriteCb<PluginsConfigureRes> cb)
{
    auto plugin_state = m_plugin_engine.Plugins().find(req.name);

    if (plugin_state == m_plugin_engine.Plugins().end())
    {
        return cb.Error(-1, "Plugin not found");
    }

    m_plugin_engine.Configure(req.name, req.config);

    cb.Ok({});
}
