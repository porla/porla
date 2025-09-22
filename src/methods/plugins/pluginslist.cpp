#include "pluginslist.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;
using porla::Lua::PluginInstallOptions;

using porla::Methods::PluginsList;
using porla::Methods::PluginsListReq;
using porla::Methods::PluginsListRes;

PluginsList::PluginsList(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsList::Invoke(const PluginsListReq& req, WriteCb<PluginsListRes> cb)
{
    PluginsListRes res = {};

    for (const auto& [ name, state ] : m_plugin_engine.Plugins())
    {
        std::string abs_path = fs::absolute(state.path);

        PluginsListRes::Plugin plugin{
            .can_configure = state.can_configure,
            .can_uninstall = state.can_uninstall,
            .name          = name,
            .path          = abs_path
        };

        res.plugins.emplace_back(std::move(plugin));
    }

    cb.Ok(res);
}
