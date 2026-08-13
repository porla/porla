#include "pluginslist.hpp"

#include <boost/log/trivial.hpp>

#include "../../data/models/plugins.hpp"
#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;

using porla::Data::Models::Plugins;
using porla::Methods::PluginsList;
using porla::Methods::PluginsListReq;
using porla::Methods::PluginsListRes;

PluginsList::PluginsList(sqlite3* db, PluginEngine& plugin_engine)
    : m_db(db)
    , m_plugin_engine(plugin_engine)
{
}

void PluginsList::Invoke(const PluginsListReq& req, WriteCb<PluginsListRes> cb)
{
    PluginsListRes res = {};

    for (const auto& plugin : Data::Models::Plugins::List(m_db))
    {
        const auto instance = m_plugin_engine.Get(plugin.id);

        const auto meta = instance == nullptr
            ? std::nullopt
            : instance->GetMeta();

        res.plugins.emplace_back(PluginsListRes::Plugin{
            .id        = plugin.id,
            .path      = plugin.path,
            .name      = meta.has_value() ? meta->name : std::nullopt,
            .version   = meta.has_value() ? meta->version : std::nullopt,
            .metadata  = plugin.metadata,
            .is_loaded = instance != nullptr
        });
    }

    cb.Ok(res);
}
