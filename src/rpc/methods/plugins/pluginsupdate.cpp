#include "pluginsupdate.hpp"

#include <boost/log/trivial.hpp>
#include <filesystem>

#include "../../../data/models/plugins.hpp"
#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginengine.hpp"

namespace fs = std::filesystem;

using porla::Data::Models::Plugins;
using porla::Lua::PluginEngine;
using porla::Rpc::Methods::Plugins::PluginsUpdate;
using porla::Rpc::Methods::Plugins::PluginsUpdateReq;
using porla::Rpc::Methods::Plugins::PluginsUpdateRes;

PluginsUpdate::PluginsUpdate(sqlite3* db, porla::Lua::PluginEngine& plugin_engine)
    : m_db(db)
    , m_plugin_engine(plugin_engine)
{
}

void PluginsUpdate::Execute(const PluginsUpdateReq& req, ResponseWriterHandle cb)
{
    auto plugin = Data::Models::Plugins::GetById(m_db, req.id);

    if (!plugin.has_value())
    {
        return cb->Error(-1, "Plugin not found");
    }

    const fs::path plugin_path = req.path;

    if (!plugin_path.is_absolute())
    {
        return cb->Error(-2, "Plugin path must be absolute");
    }

    if (!fs::exists(plugin_path))
    {
        return cb->Error(-3, "Plugin path does not exist");
    }

    plugin->config   = req.config;
    plugin->path     = plugin_path.lexically_normal();

    Data::Models::Plugins::Update(m_db, *plugin);

    m_plugin_engine.Reload(req.id);

    cb->Ok(PluginsUpdateRes{});
}
