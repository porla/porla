#include "pluginsreload.hpp"

#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;

using porla::Rpc::Methods::Plugins::PluginsReload;
using porla::Rpc::Methods::Plugins::PluginsReloadReq;
using porla::Rpc::Methods::Plugins::PluginsReloadRes;

PluginsReload::PluginsReload(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsReload::Execute(const PluginsReloadReq& req, ResponseWriterHandle cb)
{
    m_plugin_engine.Reload(req.id, [cb]()
    {
        cb->Ok({});
    });
}
