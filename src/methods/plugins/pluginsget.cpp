#include "pluginsget.hpp"

#include <boost/log/trivial.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;
using porla::Lua::PluginInstallOptions;

using porla::Methods::PluginsGet;
using porla::Methods::PluginsGetReq;
using porla::Methods::PluginsGetRes;

PluginsGet::PluginsGet(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsGet::Invoke(const PluginsGetReq& req, WriteCb<PluginsGetRes> cb)
{
    const auto plugin_state = m_plugin_engine.Plugins().find(req.name);

    if (plugin_state == m_plugin_engine.Plugins().end())
    {
        return cb.Error(-1, "Plugin not found");
    }

    std::vector<std::string> branches;
    std::vector<std::string> tags;

    cb.Ok(PluginsGetRes{
        .config = plugin_state->second.config,
        .path   = fs::absolute(plugin_state->second.path),
        .tags   = tags
    });
}
