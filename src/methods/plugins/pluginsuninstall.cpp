#include "pluginsuninstall.hpp"

#include <boost/log/trivial.hpp>
#include <git2.h>

#include "../../lua/plugins/plugin.hpp"
#include "../../lua/plugins/pluginengine.hpp"

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
    auto plugin = m_plugin_engine.Plugins().find(req.name);

    if (plugin == m_plugin_engine.Plugins().end())
    {
        return cb.Error(-1, "Plugin not found");
    }

    const auto plugin_path = plugin->second.path;
    const auto is_repository = git_repository_open_ext(
        nullptr,
        plugin_path.c_str(),
        GIT_REPOSITORY_OPEN_NO_SEARCH,
        nullptr) == GIT_OK;

    std::error_code ec;
    m_plugin_engine.Uninstall(req.name, ec);

    if (ec)
    {
        return cb.Error(-1, "Failed to uninstall plugin");
    }

    if (is_repository)
    {
        BOOST_LOG_TRIVIAL(info) << "Plugin was a Git repository - removing plugin dir";
        fs::remove_all(plugin_path, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to remove plugin directory: " << ec.message();
            return cb.Error(-2, "Plugin was uninstalled, but plugin directory could not be removed");
        }
    }

    cb.Ok({});
}
