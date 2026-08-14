#include "pluginsinstall.hpp"

#include <sstream>

#include <boost/log/trivial.hpp>

#include "../../../curlmulti.hpp"
#include "../../../data/models/plugins.hpp"
#include "../../../lua/plugin.hpp"
#include "../../../lua/pluginengine.hpp"

using porla::Data::Models::Plugins;
using porla::Lua::PluginEngine;
using porla::Rpc::Methods::Plugins::PluginsInstall;
using porla::Rpc::Methods::Plugins::PluginsInstallReq;
using porla::Rpc::Methods::Plugins::PluginsInstallRes;

PluginsInstall::PluginsInstall(sqlite3* db, std::weak_ptr<CurlMulti> cm, PluginEngine& plugin_engine)
    : m_db(db)
    , m_cm(cm)
    , m_plugin_engine(plugin_engine)
{
}

void PluginsInstall::Execute(const PluginsInstallReq& req, ResponseWriterHandle cb)
{
    auto curl = m_cm.lock();

    if (curl == nullptr)
    {
        return cb->Error(-1, "Failed to lock CurlMulti");
    }

    std::stringstream url;
    url << "https://api.github.com/repos/" << req.owner << "/" << req.repository << "/releases/tags/" << req.version;

    BOOST_LOG_TRIVIAL(trace) << "Begin installation of plugin from " << url.str();

    curl->HttpGet(url.str(), [cm = m_cm, cb](int status, std::string body)
    {
        const auto release = nlohmann::json::parse(body);

        BOOST_LOG_TRIVIAL(debug) << "GitHub release JSON: " << release;

        std::string tag_name = release["tag_name"];
        std::string download_url = release["assets"][0]["browser_download_url"];

        BOOST_LOG_TRIVIAL(info) << "Found version " << tag_name << " of plugin - fetching from " << download_url;

        auto curl = cm.lock();

        if (curl == nullptr)
        {
            cb->Error(-1, "Failed to lock CurlMulti");
            return;
        }

        curl->HttpGet(download_url, [cb](int status, std::string body)
        {
            BOOST_LOG_TRIVIAL(info) << "Plugin archive fetched. Installing.";

            cb->Ok({});
        });
    });
}
