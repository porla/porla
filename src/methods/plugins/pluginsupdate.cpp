#include "pluginsupdate.hpp"

#include <boost/log/trivial.hpp>

#include "../../data/models/plugins.hpp"
#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Data::Models::Plugins;
using porla::Lua::PluginEngine;
using porla::Methods::PluginsUpdate;
using porla::Methods::PluginsUpdateReq;
using porla::Methods::PluginsUpdateRes;

PluginsUpdate::PluginsUpdate(sqlite3* db, porla::Lua::PluginEngine& plugin_engine)
    : m_db(db)
    , m_plugin_engine(plugin_engine)
{
}

void PluginsUpdate::Invoke(const PluginsUpdateReq& req, WriteCb<PluginsUpdateRes> cb)
{
    auto plugin = Plugins::GetById(m_db, req.id);

    if (!plugin.has_value())
    {
        return cb.Error(-1, "Plugin not found");
    }

    Plugins::Update(
        m_db,
        plugin->id,
        req.config,
        req.metadata.value_or(plugin->metadata));

    auto write = std::make_shared<WriteCb<PluginsUpdateRes>>(std::move(cb));

    m_plugin_engine.Reload(req.id, [write]()
    {
        write->Ok(PluginsUpdateRes{});
    });
}
