#include "pluginsinstall.hpp"

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
using porla::Rpc::Methods::Plugins::PluginsInstall;
using porla::Rpc::Methods::Plugins::PluginsInstallReq;
using porla::Rpc::Methods::Plugins::PluginsInstallRes;

PluginsInstall::PluginsInstall(sqlite3* db, std::weak_ptr<CurlMulti> cm, PluginEngine& plugin_engine, const std::filesystem::path& state_dir)
    : m_db(db)
    , m_cm(cm)
    , m_plugin_engine(plugin_engine)
    , m_state_dir(state_dir)
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

    curl->HttpGet(url.str(), [w = weak_from_this(), cb, req](int status, std::string body)
    {
        auto self = w.lock();

        if (self == nullptr)
        {
            return cb->Error(-1, "Failed to lock this");
        }

        const auto release = nlohmann::json::parse(body);

        BOOST_LOG_TRIVIAL(debug) << "GitHub release JSON: " << release;

        std::string tag_name = release["tag_name"];
        std::string download_url = release["assets"][0]["browser_download_url"];

        BOOST_LOG_TRIVIAL(info) << "Found version " << tag_name << " of plugin - fetching from " << download_url;

        auto curl = self->m_cm.lock();

        if (curl == nullptr)
        {
            cb->Error(-1, "Failed to lock CurlMulti");
            return;
        }

        curl->HttpGet(download_url, [cb, w, req, tag_name](int status, std::string body)
        {
            auto self = w.lock();

            if (self == nullptr)
            {
                cb->Error(-1, "Failed to lock self");
                return;
            }

            BOOST_LOG_TRIVIAL(info) << "Plugin archive fetched. Installing.";

            std::stringstream zip_file_name;
            zip_file_name << req.owner << "_" << req.repository << "_" << tag_name << ".zip";

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

            const auto plugin_id = Data::Models::Plugins::Insert(
                self->m_db,
                Data::Models::Plugins::Plugin{
                    .id       = -1,
                    .path     = plugin_zip,
                    .config   = req.config,
                    .metadata = {
                        {"source", "github"},
                        {"repository", req.repository},
                        {"owner", req.owner},
                        {"version", tag_name}
                    }
                });

            BOOST_LOG_TRIVIAL(info) << "Plugin " << plugin_id << " installed with path " << plugin_zip;

            self->m_plugin_engine.Load(plugin_id);

            cb->Ok(PluginsInstallRes{
                .id = plugin_id
            });
        });
    });
}
