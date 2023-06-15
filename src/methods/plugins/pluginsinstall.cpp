#include "pluginsinstall.hpp"

#include "../../lua/plugins/plugin.hpp"
#include "../../lua/plugins/pluginengine.hpp"

using porla::Lua::Plugins::PluginEngine;
using porla::Lua::Plugins::PluginInstallOptions;

using porla::Methods::PluginsInstall;
using porla::Methods::PluginsInstallReq;
using porla::Methods::PluginsInstallRes;

PluginsInstall::PluginsInstall(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsInstall::Invoke(const PluginsInstallReq& req, WriteCb<PluginsInstallRes> cb)
{
    const PluginInstallOptions install_options{
        .config = req.config,
        .enable = req.enable.value_or(false),
        .path   = req.path
    };

    std::error_code ec;
    m_plugin_engine.Install(install_options, ec);

    if (ec)
    {
        return cb.Error(-1, "Failed to install plugin");
    }

    cb.Ok({});
}
