#include "pluginsupdate.hpp"

#include <boost/log/trivial.hpp>
#include <filesystem>

#include "../../../plugins.hpp"

namespace fs = std::filesystem;

using porla::Rpc::Methods::Plugins::PluginsUpdate;
using porla::Rpc::Methods::Plugins::PluginsUpdateReq;
using porla::Rpc::Methods::Plugins::PluginsUpdateRes;

PluginsUpdate::PluginsUpdate(porla::Plugins& plugins)
    : m_plugins(plugins)
{
}

void PluginsUpdate::Execute(const PluginsUpdateReq& req, ResponseWriterHandle cb)
{
    auto plugin = m_plugins.Get(req.id);

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

    m_plugins.Update(*plugin);

    cb->Ok(PluginsUpdateRes{});
}
