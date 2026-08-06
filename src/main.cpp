#include <boost/asio.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <sodium.h>

#include "cmdargs.hpp"
#include "config.hpp"
#include "curlmulti.hpp"
#include "logger.hpp"
#include "lua/pluginengine.hpp"
#include "sessions.hpp"
#include "webui.hpp"

#include "http/jsonrpchandler.hpp"
#include "http/metricshandler.hpp"
#include "http/webuihandler.hpp"

#include "methods/fsspace.hpp"
#include "methods/keyvalueget.hpp"
#include "methods/keyvalueset.hpp"
#include "methods/mmdblookup.hpp"
#include "methods/auth/authinit.hpp"
#include "methods/auth/authlogin.hpp"
#include "methods/plugins/pluginsget.hpp"
#include "methods/plugins/pluginsadd.hpp"
#include "methods/plugins/pluginslist.hpp"
#include "methods/plugins/pluginsreload.hpp"
#include "methods/plugins/pluginsremove.hpp"
#include "methods/plugins/pluginsupdate.hpp"
#include "methods/presets/presetsget.hpp"
#include "methods/presets/presetslist.hpp"
#include "methods/presets/presetsadd.hpp"
#include "methods/presets/presetsremove.hpp"
#include "methods/presets/presetsupdate.hpp"
#include "methods/sessions/sessionsadd.hpp"
#include "methods/sessions/sessionsget.hpp"
#include "methods/sessions/sessionslist.hpp"
#include "methods/sessions/sessionspause.hpp"
#include "methods/sessions/sessionsremove.hpp"
#include "methods/sessions/sessionsresume.hpp"
#include "methods/sessions/sessionssettingsget.hpp"
#include "methods/sessions/sessionssettingsset.hpp"
#include "methods/sessions/sessionsupdate.hpp"
#include "methods/sysstatus.hpp"
#include "methods/sysversions.hpp"
#include "methods/torrents/torrentsadd.hpp"
#include "methods/torrents/torrentscount.hpp"
#include "methods/torrents/torrentsfileslist.hpp"
#include "methods/torrents/torrentsfilesprogress.hpp"
#include "methods/torrents/torrentsget.hpp"
#include "methods/torrents/torrentslist.hpp"
#include "methods/torrents/torrentsmove.hpp"
#include "methods/torrents/torrentspause.hpp"
#include "methods/torrents/torrentspeersadd.hpp"
#include "methods/torrents/torrentspeerslist.hpp"
#include "methods/torrents/torrentsrecheck.hpp"
#include "methods/torrents/torrentsremove.hpp"
#include "methods/torrents/torrentsresume.hpp"
#include "methods/torrents/torrentspropertiesget.hpp"
#include "methods/torrents/torrentspropertiesset.hpp"
#include "methods/torrents/torrentstrackerslist.hpp"
#include "methods/webui/webuiinstall.hpp"

int main(int argc, char* argv[])
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    const boost::program_options::variables_map cmd = porla::CmdArgs::Parse(argc, argv);

    if (cmd.count("help"))
    {
        return porla::CmdArgs::Help();
    }

    porla::Logger::Setup(cmd);

    std::unique_ptr<porla::Config> cfg;

    try
    {
        cfg = porla::Config::Load(cmd);
    }
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to load configuration: " << ex.what();
        return -1;
    }

    boost::asio::io_context io;
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);

    auto curl_multi_instance = porla::CurlMulti::Create(io);

    {
        porla::Sessions sessions(porla::SessionsOptions{
            .db = cfg->db,
            .io = io
        });

        sessions.LoadAll();

        porla::Lua::PluginEngine plugin_engine{porla::Lua::PluginEngineOptions{
            .config     = *cfg,
            .curl_multi = curl_multi_instance,
            .db         = cfg->db,
            .io         = io,
            .sessions   = sessions
        }};

        plugin_engine.LoadAll();

        signals.async_wait(
            [&io, &plugin_engine, &signals](boost::system::error_code const& ec, int signal)
            {
                BOOST_LOG_TRIVIAL(info) << "Interrupt received (" << signal << ") - stopping... Press Ctrl+C again to force";

                plugin_engine.UnloadAll([&io]()
                {
                    io.stop();
                });

                signals.async_wait(
                    [&io](boost::system::error_code const& ec, int signal)
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Second interrupt received (" << signal << ") - forcing shutdown";
                        io.stop();
                    });
            });

        boost::signals2::signal<void(const char*, size_t)> webui_installed_signal;

        porla::Http::JsonRpcHandler rpc(cfg->secret_key, {
            {"auth.init", porla::Methods::Auth::AuthInit(cfg->db)},
            {"auth.login", porla::Methods::Auth::AuthLogin(cfg->db, cfg->secret_key)},
            {"fs.space", porla::Methods::FsSpace()},
            {"kv.get", porla::Methods::KeyValueGet(cfg->db)},
            {"kv.set", porla::Methods::KeyValueSet(cfg->db)},
            {"mmdb.lookup", porla::Methods::MmdbLookup(cfg->mmdb_file)},
            {"plugins.add", porla::Methods::PluginsAdd(cfg->db, plugin_engine)},
            {"plugins.get", porla::Methods::PluginsGet(cfg->db, plugin_engine)},
            {"plugins.list", porla::Methods::PluginsList(cfg->db, plugin_engine)},
            {"plugins.reload", porla::Methods::PluginsReload(plugin_engine)},
            {"plugins.remove", porla::Methods::PluginsRemove(cfg->db, plugin_engine)},
            {"plugins.update", porla::Methods::PluginsUpdate(cfg->db, plugin_engine)},
            {"presets.add", porla::Methods::Presets::PresetsAdd(cfg->db)},
            {"presets.get", porla::Methods::Presets::PresetsGet(cfg->db)},
            {"presets.list", porla::Methods::Presets::PresetsList(cfg->db)},
            {"presets.remove", porla::Methods::Presets::PresetsRemove(cfg->db)},
            {"presets.update", porla::Methods::Presets::PresetsUpdate(cfg->db)},
            {"sessions.add", porla::Methods::Sessions::SessionsAdd(cfg->db, sessions)},
            {"sessions.get", porla::Methods::Sessions::SessionsGet(cfg->db, sessions)},
            {"sessions.list", porla::Methods::SessionsList(cfg->db, sessions)},
            {"sessions.pause", porla::Methods::SessionsPause(cfg->db, sessions)},
            {"sessions.remove", porla::Methods::Sessions::SessionsRemove(cfg->db, sessions)},
            {"sessions.resume", porla::Methods::SessionsResume(cfg->db, sessions)},
            {"sessions.settings.get", porla::Methods::Sessions::SessionsSettingsGet(cfg->db, sessions)},
            {"sessions.settings.set", porla::Methods::Sessions::SessionsSettingsSet(cfg->db, sessions)},
            {"sessions.update", porla::Methods::Sessions::SessionsUpdate(cfg->db, sessions)},
            {"sys.status", porla::Methods::SysStatus(cfg->db)},
            {"sys.versions", porla::Methods::SysVersions()},
            {"torrents.add", porla::Methods::TorrentsAdd(cfg->db, sessions)},
            {"torrents.count", porla::Methods::Torrents::TorrentsCount(sessions)},
            {"torrents.files.list", porla::Methods::TorrentsFilesList(cfg->db, sessions)},
            {"torrents.files.progress", porla::Methods::TorrentsFilesProgress(cfg->db, sessions)},
            {"torrents.get", porla::Methods::TorrentsGet(cfg->db, sessions)},
            {"torrents.list", porla::Methods::TorrentsList(cfg->db, sessions)},
            {"torrents.move", porla::Methods::TorrentsMove(cfg->db, sessions)},
            {"torrents.pause", porla::Methods::TorrentsPause(cfg->db, sessions)},
            {"torrents.peers.add", porla::Methods::TorrentsPeersAdd(cfg->db, sessions)},
            {"torrents.peers.list", porla::Methods::TorrentsPeersList(cfg->db, sessions)},
            {"torrents.properties.get", porla::Methods::TorrentsPropertiesGet(cfg->db, sessions)},
            {"torrents.properties.set", porla::Methods::TorrentsPropertiesSet(cfg->db, sessions)},
            {"torrents.recheck", porla::Methods::TorrentsRecheck(cfg->db, sessions)},
            {"torrents.remove", porla::Methods::TorrentsRemove(cfg->db, sessions)},
            {"torrents.resume", porla::Methods::TorrentsResume(cfg->db, sessions)},
            {"torrents.trackers.list", porla::Methods::TorrentsTrackersList(cfg->db, sessions)},
            {"webui.install", porla::Methods::WebUI::WebUIInstall(*cfg, webui_installed_signal)}
        });

        std::string http_base_path = cfg->http_base_path.value_or("/");
        if (http_base_path.empty())        http_base_path = "/";
        if (http_base_path[0] != '/')      http_base_path = "/" + http_base_path;
        if (http_base_path.ends_with("/")) http_base_path = http_base_path.substr(0, http_base_path.size() - 1);

        uWS::Loop::get(&io);

        uWS::App http_server;
        http_server.post(http_base_path + "/api/v1/jsonrpc", rpc);

        if (cfg->http_metrics_enabled.value_or(true))
        {
            BOOST_LOG_TRIVIAL(info) << "Enabling HTTP metrics endpoint";
            http_server.get(http_base_path + "/metrics", porla::Http::MetricsHandler(sessions));
        }

        if (cfg->http_webui_enabled.value_or(true))
        {
            const auto webui_file = cfg->state_dir.value_or(fs::current_path()) / cfg->http_webui_file.value_or("webui.zip");

            if (!fs::exists(webui_file))
            {
                porla::WebUI::Download(
                    cfg->http_webui_repository.value_or("porla/web"),
                    webui_file);
            }

            if (fs::exists(webui_file))
            {
                BOOST_LOG_TRIVIAL(info) << "Enabling HTTP web UI";
                http_server.get(http_base_path + "/*", porla::Http::WebUIHandler(webui_file, http_base_path, webui_installed_signal));
            }
        }

        http_server.listen(
            cfg->http_host.value_or("127.0.0.1"),
            cfg->http_port.value_or(1337),
            [](const auto* t)
            {
                BOOST_LOG_TRIVIAL(info) << "HTTP server listening";
            });

        io.run();

        plugin_engine.UnloadAll();
    }

    curl_global_cleanup();

    BOOST_LOG_TRIVIAL(info) << "Bye";

    return 0;
}
