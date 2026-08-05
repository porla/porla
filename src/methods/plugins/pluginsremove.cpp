#include "pluginsremove.hpp"

#include "../../data/models/plugins.hpp"
#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Data::Models::Plugins;
using porla::Lua::PluginEngine;
using porla::Methods::PluginsRemove;
using porla::Methods::PluginsRemoveReq;
using porla::Methods::PluginsRemoveRes;

PluginsRemove::PluginsRemove(sqlite3* db, PluginEngine& plugin_engine)
    : m_db(db)
    , m_plugin_engine(plugin_engine)
{
}

void PluginsRemove::Invoke(const PluginsRemoveReq& req, WriteCb<PluginsRemoveRes> cb)
{
    const auto plugin = Plugins::GetById(m_db, req.id);

    if (!plugin.has_value())
    {
        return cb.Error(-1, "Plugin not found");
    }

    Plugins::Remove(m_db, req.id);

    auto write = std::make_shared<WriteCb<PluginsRemoveRes>>(std::move(cb));

    m_plugin_engine.Unload(req.id, [write]()
    {
        write->Ok({});
    });
}
