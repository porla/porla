#include "pluginsuninstall.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;
using porla::Lua::PluginInstallOptions;

using porla::Methods::PluginsUninstall;
using porla::Methods::PluginsUninstallReq;
using porla::Methods::PluginsUninstallRes;

PluginsUninstall::PluginsUninstall(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsUninstall::Invoke(const PluginsUninstallReq& req, WriteCb<PluginsUninstallRes> cb)
{
    auto plugin = m_plugin_engine.Plugins().find(req.name);

    if (plugin == m_plugin_engine.Plugins().end())
    {
        return cb.Error(-1, "Plugin not found");
    }

    std::error_code ec;
    m_plugin_engine.Uninstall(req.name, ec);

    if (ec)
    {
        return cb.Error(-1, "Failed to uninstall plugin");
    }

    cb.Ok({});
}
