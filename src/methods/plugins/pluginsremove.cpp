#include "pluginsremove.hpp"

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;

using porla::Methods::PluginsRemove;
using porla::Methods::PluginsRemoveReq;
using porla::Methods::PluginsRemoveRes;

PluginsRemove::PluginsRemove(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsRemove::Invoke(const PluginsRemoveReq& req, WriteCb<PluginsRemoveRes> cb)
{
    auto write = std::make_shared<WriteCb<PluginsRemoveRes>>(std::move(cb));

    m_plugin_engine.Uninstall(req.id, [write]()
    {
        write->Ok({});
    });
}
