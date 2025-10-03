#include "pluginsuninstall.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;

using porla::Methods::PluginsUninstall;
using porla::Methods::PluginsUninstallReq;
using porla::Methods::PluginsUninstallRes;

PluginsUninstall::PluginsUninstall(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsUninstall::Invoke(const PluginsUninstallReq& req, WriteCb<PluginsUninstallRes> cb)
{
    m_plugin_engine.Uninstall(req.id);
    cb.Ok({});
}
