#include "pluginslist.hpp"

#include <boost/log/trivial.hpp>
#include <toml++/toml.hpp>

#include "../../../data/models/plugins.hpp"
#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginengine.hpp"
#include "../../../lua/pluginsource.hpp"

using porla::Data::Models::Plugins;
using porla::Lua::PluginEngine;
using porla::Rpc::Methods::Plugins::PluginsList;
using porla::Rpc::Methods::Plugins::PluginsListReq;
using porla::Rpc::Methods::Plugins::PluginsListRes;

PluginsList::PluginsList(sqlite3* db, PluginEngine& plugin_engine)
    : m_db(db)
    , m_plugin_engine(plugin_engine)
{
}

void PluginsList::Execute(const PluginsListReq& req, ResponseWriterHandle cb)
{
    PluginsListRes res = {};

    for (const auto& plugin : Data::Models::Plugins::List(m_db))
    {
        const auto instance    = m_plugin_engine.Get(plugin.id);
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
