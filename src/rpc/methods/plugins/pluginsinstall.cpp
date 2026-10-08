#include "pluginsinstall.hpp"

#include <fstream>
#include <sstream>

#include <boost/log/trivial.hpp>

#include "../../../curlmulti.hpp"
#include "../../../json/github.hpp"
#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginengine.hpp"
#include "../../../plugins.hpp"

namespace fs = std::filesystem;

using porla::Json::GitHubRelease;
using porla::Lua::PluginEngine;
using porla::Rpc::Methods::Plugins::PluginsInstall;
using porla::Rpc::Methods::Plugins::PluginsInstallReq;
using porla::Rpc::Methods::Plugins::PluginsInstallRes;

PluginsInstall::PluginsInstall(boost::asio::io_context& io, sqlite3* db, std::weak_ptr<CurlMulti> cm, porla::Plugins& plugins, const std::filesystem::path& state_dir)
    : TypedAsyncMethod(io.get_executor())
    , m_db(db)
    , m_cm(cm)
    , m_plugins(plugins)
    , m_state_dir(state_dir)
{
}

boost::asio::awaitable<void> PluginsInstall::ExecuteAsync(PluginsInstallReq req, ResponseWriterHandle cb)
{
    auto curl = m_cm.lock();

    if (curl == nullptr)
    {
        co_return cb->Error(-99, "Failed to lock state");
    }

    std::stringstream url;
    url << "https://api.github.com/repos/" << req.owner << "/" << req.repository << "/releases/tags/" << req.version;

    BOOST_LOG_TRIVIAL(trace) << "Begin installation of plugin from " << url.str();

    const auto [
        release_status,
        release_body ] = co_await curl->AsyncHttpGet(url.str(), boost::asio::use_awaitable);

    if (release_status != 200)
    {
        co_return cb->Error(-2, "Release not found - HTTP status: " + std::to_string(release_status));
    }

    GitHubRelease release;

    try
    {
        release = nlohmann::json::parse(release_body).get<GitHubRelease>();
    }
    catch (const std::exception& e)
    {
        co_return cb->Error(-3, "Failed to parse release body as JSON", {{"what", e.what()}});
    }

    if (release.assets.size() == 0)
    {
        co_return cb->Error(-4, "Release has no downloadable asset");
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
        co_return cb->Error(-5, "Failed to fetch release asset");
    }

    std::stringstream zip_file_name;
    zip_file_name << req.owner << "_" << req.repository << "_" << release.tag_name << ".zip";

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
            co_return cb->Error(-6, "Failed to write plugin archive to " + plugin_zip.string());
        }

        BOOST_LOG_TRIVIAL(debug) << "Wrote plugin to " << plugin_zip;
    }

    const auto plugin_id = m_plugins.Add(porla::Plugins::Plugin{
        .id       = -1,
        .path     = plugin_zip,
        .config   = req.config,
        .metadata = {
            {"source", "github"},
            {"owner", req.owner},
            {"repository", req.repository},
            {"version", release.tag_name}
        }
    });

    cb->Ok(PluginsInstallRes{
        .id = plugin_id
    });
}
