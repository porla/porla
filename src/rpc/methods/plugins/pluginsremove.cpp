#include "pluginsremove.hpp"

#include "../../../data/models/plugins.hpp"
#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginengine.hpp"

using porla::Data::Models::Plugins;
using porla::Lua::PluginEngine;
using porla::Rpc::Methods::Plugins::PluginsRemove;
using porla::Rpc::Methods::Plugins::PluginsRemoveReq;
using porla::Rpc::Methods::Plugins::PluginsRemoveRes;

PluginsRemove::PluginsRemove(sqlite3* db, PluginEngine& plugin_engine)
    : m_db(db)
    , m_plugin_engine(plugin_engine)
{
}

void PluginsRemove::Execute(const PluginsRemoveReq& req, ResponseWriterHandle cb)
{
    const auto plugin = Data::Models::Plugins::GetById(m_db, req.id);

    if (!plugin.has_value())
    {
        return cb->Error(-1, "Plugin not found");
    }

    Data::Models::Plugins::Remove(m_db, req.id);

    m_plugin_engine.Unload(req.id, [cb]()
    {
        cb->Ok({});
    });
}
