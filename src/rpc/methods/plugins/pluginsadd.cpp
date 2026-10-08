#include "pluginsadd.hpp"

#include <filesystem>

#include <boost/log/trivial.hpp>

#include "../../../plugins.hpp"

#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginengine.hpp"

namespace fs = std::filesystem;

using porla::Data::Models::Plugins;
using porla::Lua::PluginEngine;
using porla::Rpc::Methods::Plugins::PluginsAdd;
using porla::Rpc::Methods::Plugins::PluginsAddReq;
using porla::Rpc::Methods::Plugins::PluginsAddRes;

PluginsAdd::PluginsAdd(porla::Plugins& plugins)
    : m_plugins(plugins)
{
}

void PluginsAdd::Execute(const PluginsAddReq& req, ResponseWriterHandle cb)
{
    fs::path plugin_path = req.path;

    if (!plugin_path.is_absolute())
    {
        return cb->Error(-2, "Plugin path must be absolute");
    }

    if (!fs::exists(plugin_path))
    {
        return cb->Error(-3, "Plugin path does not exist");
    }

    const auto plugin_id = m_plugins.Add(porla::Plugins::Plugin{
        .id       = -1,
        .path     = plugin_path,
        .config   = req.config,
        .metadata = {}
    });

    return cb->Ok(PluginsAddRes{
        .id = plugin_id
    });
}
