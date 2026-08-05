#include "pluginsget.hpp"

#include <boost/log/trivial.hpp>

#include "../../data/models/plugins.hpp"
#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Data::Models::Plugins;
using porla::Lua::PluginEngine;
using porla::Methods::PluginsGet;
using porla::Methods::PluginsGetReq;
using porla::Methods::PluginsGetRes;

PluginsGet::PluginsGet(sqlite3* db, PluginEngine& plugin_engine)
    : m_db(db)
    , m_plugin_engine(plugin_engine)
{
}

void PluginsGet::Invoke(const PluginsGetReq& req, WriteCb<PluginsGetRes> cb)
{
    const auto plugin = Plugins::GetById(m_db, req.id);

    if (!plugin.has_value())
    {
        return cb.Error(-1, "Plugin not found");
    }

    const auto instance = m_plugin_engine.Get(req.id);

    const auto meta = instance == nullptr
        ? std::nullopt
        : instance->GetMeta();

    return cb.Ok(PluginsGetRes{
        .plugin = PluginsGetRes::Plugin{
            .id        = plugin->id,
            .type      = plugin->type,
            .name      = meta.has_value() ? meta->name    : std::nullopt,
            .version   = meta.has_value() ? meta->version : std::nullopt,
            .config    = plugin->config,
            .metadata  = plugin->metadata,
            .is_loaded = instance != nullptr
        }
    });
}
