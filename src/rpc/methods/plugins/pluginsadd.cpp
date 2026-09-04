#include "pluginsadd.hpp"

#include <filesystem>

#include <boost/log/trivial.hpp>

#include "../../../data/models/plugins.hpp"

#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginengine.hpp"

namespace fs = std::filesystem;

using porla::Data::Models::Plugins;
using porla::Lua::PluginEngine;
using porla::Rpc::Methods::Plugins::PluginsAdd;
using porla::Rpc::Methods::Plugins::PluginsAddReq;
using porla::Rpc::Methods::Plugins::PluginsAddRes;

PluginsAdd::PluginsAdd(sqlite3* db, porla::Lua::PluginEngine& plugins)
    : m_db(db)
    , m_plugins(plugins)
{
}

void PluginsAdd::Execute(const PluginsAddReq& req, ResponseWriterHandle cb)
{
    fs::path plugin_path = req.path;

    if (!plugin_path.is_absolute())
    {
        return cb->Error(-101, "Plugin path must be absolute");
    }

    if (!fs::exists(plugin_path))
    {
        return cb->Error(-102, "Plugin path does not exist");
    }

    const auto plugin_id = Data::Models::Plugins::Insert(
        m_db,
        Data::Models::Plugins::Plugin{
            .id       = -1,
            .path     = plugin_path,
            .config   = req.config,
            .metadata = req.metadata.value_or({})
        });

    BOOST_LOG_TRIVIAL(info) << "Plugin " << plugin_id << " installed with path " << plugin_path;

    m_plugins.Load(plugin_id);

    return cb->Ok(PluginsAddRes{
        .id = plugin_id
    });
}
