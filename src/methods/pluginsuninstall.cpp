#include "pluginsuninstall.hpp"

#include "../lua/plugins/plugin.hpp"
#include "../lua/plugins/pluginengine.hpp"

using porla::Lua::Plugins::PluginEngine;
using porla::Lua::Plugins::PluginInstallOptions;

using porla::Methods::PluginsUninstall;
using porla::Methods::PluginsUninstallReq;
using porla::Methods::PluginsUninstallRes;

PluginsUninstall::PluginsUninstall(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsUninstall::Invoke(const PluginsUninstallReq& req, WriteCb<PluginsUninstallRes> cb)
{
    std::error_code ec;
    m_plugin_engine.Uninstall(req.name, ec);

    if (ec)
    {
        return cb.Error(-1, "Failed to uninstall plugin");
    }

    cb.Ok({});
}
