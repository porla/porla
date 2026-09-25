#include "pluginsupgrade.hpp"

#include <fstream>
#include <sstream>

#include <boost/log/trivial.hpp>

#include "../../../curlmulti.hpp"
#include "../../../data/models/plugins.hpp"
#include "../../../json/github.hpp"
#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginengine.hpp"

namespace fs = std::filesystem;

using porla::Data::Models::Plugins;
using porla::Json::GitHubRelease;
using porla::Lua::PluginEngine;
using porla::Rpc::Methods::Plugins::PluginsUpgrade;
using porla::Rpc::Methods::Plugins::PluginsUpgradeReq;
using porla::Rpc::Methods::Plugins::PluginsUpgradeRes;

PluginsUpgrade::PluginsUpgrade(boost::asio::io_context& io, sqlite3* db, std::weak_ptr<CurlMulti> cm, PluginEngine& plugin_engine, const std::filesystem::path& state_dir)
    : TypedAsyncMethod(io.get_executor())
    , m_db(db)
    , m_cm(cm)
    , m_plugin_engine(plugin_engine)
    , m_state_dir(state_dir)
{
}

boost::asio::awaitable<void> PluginsUpgrade::ExecuteAsync(PluginsUpgradeReq req, ResponseWriterHandle cb)
{
    auto curl = m_cm.lock();
    if (curl == nullptr) { co_return cb->Error(-99, "Failed to lock state"); }

    const auto plugin = Data::Models::Plugins::GetById(m_db, req.id);

    if (!plugin.has_value())
    {
        co_return cb->Error(-1, "Plugin not found");
    }

    const auto metadata_lookup = [&](const char* key) -> std::optional<std::string>
    {
        const auto it = plugin->metadata.find(key);
        if (it == plugin->metadata.end() || !it->second.is_string()) { return std::nullopt; }
        return it->second.get<std::string>();
    };

    const auto owner      = metadata_lookup("owner");
    const auto repository = metadata_lookup("repository");
    const auto version    = metadata_lookup("version");

    if (!metadata_lookup("source") || !owner || !repository || !version)
    {
        co_return cb->Error(-2, "Plugin is missing required metadata for upgrading");
    }

    if (*version == req.version)
    {
        co_return cb->Error(-3, "Cannot upgrade to same version");
    }

    std::stringstream url;
    url << "https://api.github.com/repos/"
        << owner.value()
        << "/"
        << repository.value()
        << "/releases/tags/" << req.version;

    BOOST_LOG_TRIVIAL(trace) << "Begin upgrading plugin from " << url.str();

    const auto [
        release_status,
        release_body ] = co_await curl->AsyncHttpGet(url.str(), boost::asio::use_awaitable);

    if (release_status != 200)
    {
        co_return cb->Error(-4, "Release not found");
    }

    GitHubRelease release;

    try
    {
        release = nlohmann::json::parse(release_body).get<GitHubRelease>();
    }
    catch (const std::exception& e)
    {
        co_return cb->Error(-5, "Failed to parse release body as JSON", {{"what", e.what()}});
    }

    if (release.assets.size() == 0)
    {
        co_return cb->Error(-6, "Release has no downloadable asset");
    }

    BOOST_LOG_TRIVIAL(info)
        << "Found version "
        << release.tag_name
        << " of plugin - fetching from " << release.assets[0].browser_download_url;

    const auto [
        asset_status,
        asset_body ] = co_await curl->AsyncHttpGet(release.assets[0].browser_download_url, boost::asio::use_awaitable);

    if (asset_status != 200)
    {
        co_return cb->Error(-7, "Failed to fetch release asset");
    }

    std::stringstream zip_file_name;
    zip_file_name
        << owner.value()
        << "_"
        << repository.value()
        << "_" << release.tag_name << ".zip";

    const auto plugins_dir = fs::absolute(m_state_dir / "plugins").lexically_normal();
    const auto plugin_zip  = plugins_dir / zip_file_name.str();

    if (!fs::exists(plugins_dir))
    {
        fs::create_directories(plugins_dir);
    }

    {
        std::ofstream out(plugin_zip, std::ios::binary);

        out.write(
            asset_body.data(),
            asset_body.size());

        if (!out)
        {
            co_return cb->Error(-8, "Failed to write plugin archive to " + plugin_zip.string());
        }

        BOOST_LOG_TRIVIAL(debug) << "Wrote plugin to " << plugin_zip;
    }

    const auto old_path = plugin->path;

    auto metadata = plugin->metadata;
    metadata["version"] = release.tag_name;

    Data::Models::Plugins::Update(
        m_db,
        Data::Models::Plugins::Plugin{
            .id       = plugin->id,
            .path     = plugin_zip,
            .config   = plugin->config,
            .metadata = metadata
        });

    m_plugin_engine.Reload(plugin->id);

    std::error_code same_ec;
    if (!fs::equivalent(old_path, plugin_zip, same_ec) && fs::exists(old_path))
    {
        std::error_code ec;

        if (!fs::remove(old_path, ec))
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to remove old plugin path " << old_path << ": " << ec;
        }
    }

    cb->Ok(PluginsUpgradeRes{});
}
