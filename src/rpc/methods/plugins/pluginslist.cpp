#include "pluginslist.hpp"

#include <boost/log/trivial.hpp>
#include <toml++/toml.hpp>

#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginsource.hpp"
#include "../../../plugins.hpp"

using porla::Rpc::Methods::Plugins::PluginsList;
using porla::Rpc::Methods::Plugins::PluginsListReq;
using porla::Rpc::Methods::Plugins::PluginsListRes;

PluginsList::PluginsList(porla::Plugins& plugins)
    : m_plugins(plugins)
{
}

void PluginsList::Execute(const PluginsListReq& req, ResponseWriterHandle cb)
{
    PluginsListRes res = {};

    for (const auto& plugin : m_plugins.All())
    {
        const auto instance    = m_plugins.Instance(plugin.id);
        const auto plugin_path = fs::path(plugin.path);

        auto name    = std::optional<std::string>(plugin_path.filename());
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

        res.plugins.emplace_back(PluginsListRes::Plugin{
            .id        = plugin.id,
            .path      = plugin.path,
            .name      = name,
            .version   = version,
            .metadata  = plugin.metadata,
            .is_loaded = instance != nullptr
        });
    }

    cb->Ok(res);
}
