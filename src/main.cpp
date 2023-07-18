#include <boost/asio.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <git2.h>
#include <sodium.h>

#include "cmdargs.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "lua/plugins/pluginengine.hpp"
#include "session.hpp"
#include "tools/authtoken.hpp"
#include "tools/generatesecretkey.hpp"
#include "tools/versionjson.hpp"
#include "utils/secretkey.hpp"

#include "http/authinithandler.hpp"
#include "http/authloginhandler.hpp"
#include "http/eventshandler.hpp"
#include "http/jsonrpchandler.hpp"
#include "http/jwthandler.hpp"
#include "http/metricshandler.hpp"
#include "http/systemhandler.hpp"
#include "http/webuihandler.hpp"

#include "methods/fsspace.hpp"
#include "methods/plugins/pluginsconfigure.hpp"
#include "methods/plugins/pluginsget.hpp"
#include "methods/plugins/pluginsinstall.hpp"
#include "methods/plugins/pluginslist.hpp"
#include "methods/plugins/pluginsreload.hpp"
#include "methods/plugins/pluginsuninstall.hpp"
#include "methods/plugins/pluginsupdate.hpp"
#include "methods/presetslist.hpp"
#include "methods/sessionpause.hpp"
#include "methods/sessionresume.hpp"
#include "methods/sessionsettingslist.hpp"
#include "methods/sysversions.hpp"
#include "methods/torrentsadd.hpp"
#include "methods/torrentsfileslist.hpp"
#include "methods/torrentslist.hpp"
#include "methods/torrentsmetadatalist.hpp"
#include "methods/torrentsmove.hpp"
#include "methods/torrentspause.hpp"
#include "methods/torrentspeersadd.hpp"
#include "methods/torrentspeerslist.hpp"
#include "methods/torrentsrecheck.hpp"
#include "methods/torrentsremove.hpp"
#include "methods/torrentsresume.hpp"
#include "methods/torrentspropertiesget.hpp"
#include "methods/torrentspropertiesset.hpp"
#include "methods/torrentstrackerslist.hpp"

int main(int argc, char* argv[])
{
    static std::map<std::string, std::function<int(int, char**, std::unique_ptr<porla::Config>)>> subcommands =
    {
        {"auth:token", &porla::Tools::AuthToken},
        {"key:generate", &porla::Tools::GenerateSecretKey},
        {"version:json", &porla::Tools::VersionJson}
    };

    curl_global_init(CURL_GLOBAL_DEFAULT);
    git_libgit2_init();

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
        BOOST_LOG_TRIVIAL(fatal) << "Failed to load configuration";
        return -1;
    }

    // Check if we should run one of the subcommands we have.
    if (argc >= 2 && subcommands.contains(argv[1]))
    {
        return subcommands.at(argv[1])(argc, argv, std::move(cfg));
    }

    boost::asio::io_context io;
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);

    signals.async_wait(
        [&io](boost::system::error_code const& ec, int signal)
        {
            BOOST_LOG_TRIVIAL(info) << "Interrupt received (" << signal << ") - stopping...";
            io.stop();
        });

    {
        porla::Session session(io, porla::SessionOptions{
            .db                         = cfg->db,
            .extensions                 = cfg->session_extensions,
            .settings                   = cfg->session_settings,
            .session_params_file        = cfg->state_dir.value_or(fs::current_path()) / "session.dat",
            .timer_dht_stats            = cfg->timer_dht_stats.value_or(5000),
            .timer_session_stats        = cfg->timer_session_stats.value_or(5000),
            .timer_torrent_updates      = cfg->timer_torrent_updates.value_or(1000)
        });

        // Load plugins before we load the torrents to give plugins a chance to run any hooks.
        porla::Lua::Plugins::PluginEngine plugin_engine{porla::Lua::Plugins::PluginEngineOptions{
            .config  = *cfg,
            .db      = cfg->db,
            .io      = io,
            .session = session
        }};

        try
        {
            session.Load();
        }
        catch (const std::exception &ex)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to load torrents: " << ex.what();
            return -1;
        }

        const fs::path default_plugin_install_dir = cfg->state_dir.value_or(fs::path()) / "installed_plugins";

        const porla::Methods::PluginsInstallOptions plugins_install_options{
            .allow_git     = cfg->plugins_allow_git.value_or(false),
            .install_dir   = cfg->plugins_install_dir.value_or(default_plugin_install_dir),
            .io            = io,
            .plugin_engine = plugin_engine
        };

        const porla::Methods::PluginsUpdateOptions plugins_update_options{
            .io            = io,
            .plugin_engine = plugin_engine
        };

        porla::Http::JsonRpcHandler rpc({
            {"fs.space", porla::Methods::FsSpace()},
            {"plugins.configure", porla::Methods::PluginsConfigure(plugin_engine)},
            {"plugins.get", porla::Methods::PluginsGet(plugin_engine)},
            {"plugins.install", porla::Methods::PluginsInstall(plugins_install_options)},
            {"plugins.list", porla::Methods::PluginsList(plugin_engine)},
            {"plugins.reload", porla::Methods::PluginsReload(plugin_engine)},
            {"plugins.uninstall", porla::Methods::PluginsUninstall(plugin_engine)},
            {"plugins.update", porla::Methods::PluginsUpdate(plugins_update_options)},
            {"presets.list", porla::Methods::PresetsList(cfg->presets)},
            {"session.pause", porla::Methods::SessionPause(session)},
            {"session.resume", porla::Methods::SessionResume(session)},
            {"session.settings.list", porla::Methods::SessionSettingsList(session)},
            {"sys.versions", porla::Methods::SysVersions()},
            {"torrents.add", porla::Methods::TorrentsAdd(cfg->db, session, cfg->presets)},
            {"torrents.files.list", porla::Methods::TorrentsFilesList(session)},
            {"torrents.list", porla::Methods::TorrentsList(cfg->db, session)},
            {"torrents.metadata.list", porla::Methods::TorrentsMetadataList(cfg->db, session)},
            {"torrents.move", porla::Methods::TorrentsMove(session)},
            {"torrents.pause", porla::Methods::TorrentsPause(session)},
            {"torrents.peers.add", porla::Methods::TorrentsPeersAdd(session)},
            {"torrents.peers.list", porla::Methods::TorrentsPeersList(session)},
            {"torrents.properties.get", porla::Methods::TorrentsPropertiesGet(session)},
            {"torrents.properties.set", porla::Methods::TorrentsPropertiesSet(session)},
            {"torrents.recheck", porla::Methods::TorrentsRecheck(session)},
            {"torrents.remove", porla::Methods::TorrentsRemove(session)},
            {"torrents.resume", porla::Methods::TorrentsResume(session)},
            {"torrents.trackers.list", porla::Methods::TorrentsTrackersList(session)}
        });

        std::string http_base_path = cfg->http_base_path.value_or("/");
        if (http_base_path.empty())        http_base_path = "/";
        if (http_base_path[0] != '/')      http_base_path = "/" + http_base_path;
        if (http_base_path.ends_with("/")) http_base_path = http_base_path.substr(0, http_base_path.size() - 1);

        uWS::Loop::get(&io);

        uWS::App http_server;
        http_server.post(http_base_path + "/api/v1/auth/init", porla::Http::AuthInitHandler(io, cfg->db, cfg->sodium_memlimit.value_or(crypto_pwhash_MEMLIMIT_MIN)));
        http_server.post(http_base_path + "/api/v1/auth/login", porla::Http::AuthLoginHandler(porla::Http::AuthLoginHandlerOptions{
            .db         = cfg->db,
            .io         = io,
            .secret_key = cfg->secret_key
        }));

        http_server.get(http_base_path + "/api/v1/events",
            cfg->http_auth_enabled.value_or(true)
                ? static_cast<porla::Http::Handler>(porla::Http::JwtHandler(cfg->secret_key, porla::Http::EventsHandler(session)))
                : static_cast<porla::Http::Handler>(porla::Http::EventsHandler(session)));

        http_server.post(http_base_path + "/api/v1/jsonrpc",
            cfg->http_auth_enabled.value_or(true)
                ? static_cast<porla::Http::Handler>(porla::Http::JwtHandler(cfg->secret_key, rpc))
                : static_cast<porla::Http::Handler>(rpc));

        http_server.get(http_base_path + "/api/v1/system", porla::Http::SystemHandler(cfg->db));

        if (cfg->http_metrics_enabled.value_or(true))
        {
            BOOST_LOG_TRIVIAL(info) << "Enabling HTTP metrics endpoint";
            http_server.get(http_base_path + "/metrics", porla::Http::MetricsHandler(session));
        }

        if (cfg->http_webui_enabled.value_or(true))
        {
            BOOST_LOG_TRIVIAL(info) << "Enabling HTTP web UI";
            http_server.get(http_base_path + "/*", porla::Http::WebUIHandler(http_base_path));
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

    git_libgit2_shutdown();
    curl_global_cleanup();

    return 0;
}
