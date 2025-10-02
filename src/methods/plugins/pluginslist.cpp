#include "pluginslist.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;

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

    for (const auto& [ id, state ] : m_plugin_engine.Plugins())
    {
        res.plugins.emplace_back(PluginsListRes::Plugin{
        });
    }

    cb.Ok(res);
}
