#include "pluginsconfigure.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;
using porla::Lua::PluginInstallOptions;

using porla::Methods::PluginsConfigure;
using porla::Methods::PluginsConfigureReq;
using porla::Methods::PluginsConfigureRes;

template <bool SSL> PluginsConfigure<SSL>::PluginsConfigure(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

template <bool SSL> void PluginsConfigure<SSL>::Invoke(const PluginsConfigureReq& req, WriteCb<PluginsConfigureRes, SSL> cb)
{
    auto plugin_state = m_plugin_engine.Plugins().find(req.name);

    if (plugin_state == m_plugin_engine.Plugins().end())
    {
        return cb.Error(-1, "Plugin not found");
    }

    m_plugin_engine.Configure(req.name, req.config);

    cb.Ok({});
}

namespace porla::Methods {
    template class PluginsConfigure<true>;
    template class PluginsConfigure<false>;
}
