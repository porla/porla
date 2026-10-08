#include "pluginsget.hpp"

#include <boost/log/trivial.hpp>
#include <toml++/toml.hpp>

#include "../../../plugins.hpp"
#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginsource.hpp"

using porla::Rpc::Methods::Plugins::PluginsGet;
using porla::Rpc::Methods::Plugins::PluginsGetReq;
using porla::Rpc::Methods::Plugins::PluginsGetRes;

PluginsGet::PluginsGet(porla::Plugins& plugins)
    : m_plugins(plugins)
{
}

void PluginsGet::Execute(const PluginsGetReq& req, ResponseWriterHandle cb)
{
    const auto plugin = m_plugins.Get(req.id);

    if (!plugin.has_value())
    {
        return cb->Error(-1, "Plugin not found");
    }

    const auto instance = m_plugins.Instance(req.id);

    auto name     = std::optional<std::string>();
    auto version = std::optional<std::string>();

    if (instance != nullptr)
    {
        const auto& source      = instance->Source();
        const auto& plugin_toml = source.sources.find("plugin.toml");

        if (plugin_toml != source.sources.end())
        {
            const auto parsed_toml = toml::parse(plugin_toml->second.data());
            if (const auto parsed_name    = parsed_toml["name"].value<std::string>())    name    = parsed_name.value();
            if (const auto parsed_version = parsed_toml["version"].value<std::string>()) version = parsed_version.value();
        }
    }

    return cb->Ok(PluginsGetRes{
        .plugin = PluginsGetRes::Plugin{
            .id        = plugin->id,
            .path      = plugin->path,
            .name      = name,
            .version   = version,
            .config    = plugin->config,
            .metadata  = plugin->metadata,
            .is_loaded = instance != nullptr
        }
    });
}
