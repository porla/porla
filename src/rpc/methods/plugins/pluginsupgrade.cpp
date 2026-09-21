#include "pluginsupgrade.hpp"

#include <fstream>
#include <sstream>

#include <boost/log/trivial.hpp>

#include "../../../curlmulti.hpp"
#include "../../../data/models/plugins.hpp"
#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginengine.hpp"

namespace fs = std::filesystem;

using porla::Data::Models::Plugins;
using porla::Lua::PluginEngine;
using porla::Rpc::Methods::Plugins::PluginsUpgrade;
using porla::Rpc::Methods::Plugins::PluginsUpgradeReq;
using porla::Rpc::Methods::Plugins::PluginsUpgradeRes;

PluginsUpgrade::PluginsUpgrade(sqlite3* db, std::weak_ptr<CurlMulti> cm, PluginEngine& plugin_engine, const std::filesystem::path& state_dir)
    : m_db(db)
    , m_cm(cm)
    , m_plugin_engine(plugin_engine)
    , m_state_dir(state_dir)
{
}

void PluginsUpgrade::Execute(const PluginsUpgradeReq& req, ResponseWriterHandle cb)
{
    auto curl = m_cm.lock();
    if (curl == nullptr) { return cb->Error(-99, "Failed to lock state"); }

    const auto plugin = Data::Models::Plugins::GetById(m_db, req.id);

    if (!plugin.has_value())
    {
        return cb->Error(-1, "Plugin not found");
    }

    if (!plugin->metadata.contains("source")
        || !plugin->metadata.contains("owner")
        || !plugin->metadata.contains("repository"))
    {
        return cb->Error(-2, "Plugin is missing required metadata for upgrading");
    }

    std::stringstream url;
    url << "https://api.github.com/repos/"
        << plugin->metadata.at("owner").get<std::string>()
        << "/"
        << plugin->metadata.at("repository").get<std::string>()
        << "/releases/tags/" << req.version;

    BOOST_LOG_TRIVIAL(trace) << "Begin upgrading plugin from " << url.str();

    curl->HttpGet(url.str(), [w = weak_from_this(), cb, plugin, req](int status, std::string body)
    {
        if (status != 200)
        {
            return cb->Error(-3, "Release not found");
        }

        auto self = w.lock();
        if (self == nullptr) { return cb->Error(-99, "Failed to lock state"); }

        nlohmann::json release;

        try
        {
            release = nlohmann::json::parse(body);
        }
        catch (const std::exception& e)
        {
            return cb->Error(-3, "Failed to parse release body as JSON");
        }

        std::string tag_name = release["tag_name"];
        std::string download_url = release["assets"][0]["browser_download_url"];

        BOOST_LOG_TRIVIAL(info) << "Found version " << tag_name << " of plugin - fetching from " << download_url;

        auto curl = self->m_cm.lock();
        if (curl == nullptr) { return cb->Error(-99, "Failed to lock state"); }

        curl->HttpGet(download_url, [cb, w, plugin, req, tag_name](int status, std::string body)
        {
            if (status != 200)
            {
                return cb->Error(-4, "Failed to fetch release asset");
            }

            auto self = w.lock();
            if (self == nullptr) { return cb->Error(-99, "Failed to lock state"); }

            std::stringstream zip_file_name;
            zip_file_name
                << plugin->metadata.at("owner").get<std::string>()
                << "_"
                << plugin->metadata.at("repository").get<std::string>()
                << "_" << tag_name << ".zip";

            const auto plugins_dir = self->m_state_dir / "plugins";
            const auto plugin_zip  = plugins_dir / zip_file_name.str();

            if (!fs::exists(plugins_dir))
            {
                fs::create_directories(plugins_dir);
            }

            {
                std::ofstream out(plugin_zip, std::ios::binary);
                out << body;
                BOOST_LOG_TRIVIAL(debug) << "Wrote plugin to " << plugin_zip;
            }

            auto metadata = plugin->metadata;
            metadata["version"] = tag_name;

            Data::Models::Plugins::Update(
                self->m_db,
                Data::Models::Plugins::Plugin{
                    .id       = plugin->id,
                    .path     = plugin_zip,
                    .config   = plugin->config,
                    .metadata = metadata
                });

            self->m_plugin_engine.Reload(plugin->id);

            cb->Ok(PluginsUpgradeRes{});
        });
    });
}
