#include "pluginsreload.hpp"

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;

using porla::Methods::PluginsReload;
using porla::Methods::PluginsReloadReq;
using porla::Methods::PluginsReloadRes;

PluginsReload::PluginsReload(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsReload::Invoke(const PluginsReloadReq& req, WriteCb<PluginsReloadRes> cb)
{
    auto write = std::make_shared<WriteCb<PluginsReloadRes>>(std::move(cb));

    m_plugin_engine.Reload(req.id, [write]()
    {
        write->Ok({});
    });
}
