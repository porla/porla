#include <any>
#include <boost/asio.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <git2.h>
#include <sodium.h>
#include <uWebSockets/App.h>
#include <uWebSockets/Loop.h>

#include "cmdargs.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "lua/pluginengine.hpp"
#include "sessions.hpp"
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
#include "methods/sessions/sessionslist.hpp"
#include "methods/sessions/sessionspause.hpp"
#include "methods/sessions/sessionsresume.hpp"
#include "methods/sessions/sessionssettingslist.hpp"
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
        porla::Sessions sessions(porla::SessionsOptions{
            .db = cfg->db,
            .io = io
        });

        for (const auto& [name, settings] : cfg->sessions)
        {
            BOOST_LOG_TRIVIAL(info) << "Loading session " << name;

            const auto state_dir = cfg->state_dir.value_or(fs::current_path());

            const auto session_params_file_name = name == "default" ? "session.dat" : "session." + name + ".dat";
            const auto session_params_file      = state_dir / session_params_file_name;

            try
            {
                sessions.Load(porla::SessionsLoadOptions{
                    .name                = name,
                    .session_params_file = session_params_file,
                    .settings            = settings
                });
            }
            catch (const std::exception &ex)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to load session " << name << ": " << ex.what();

                if (name == "default")
                {
                    return -1;
                }
            }
        }

        // Load plugins before we load the torrents to give plugins a chance to run any hooks.
        porla::Lua::PluginEngine plugin_engine{porla::Lua::PluginEngineOptions{
            .config   = *cfg,
            .db       = cfg->db,
            .io       = io,
            .sessions = sessions
        }};

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

        porla::Http::JsonRpcHandler<true> rpcSSL({
            {"fs.space", porla::Methods::FsSpace<true>()},
            {"plugins.configure", porla::Methods::PluginsConfigure<true>(plugin_engine)},
            {"plugins.get", porla::Methods::PluginsGet<true>(plugin_engine)},
            {"plugins.install", porla::Methods::PluginsInstall<true>(plugins_install_options)},
            {"plugins.list", porla::Methods::PluginsList<true>(plugin_engine)},
            {"plugins.reload", porla::Methods::PluginsReload<true>(plugin_engine)},
            {"plugins.uninstall", porla::Methods::PluginsUninstall<true>(plugin_engine)},
            {"plugins.update", porla::Methods::PluginsUpdate<true>(plugins_update_options)},
            {"presets.list", porla::Methods::PresetsList<true>(cfg->presets)},
            {"sessions.list", porla::Methods::SessionsList<true>(sessions)},
            {"sessions.pause", porla::Methods::SessionsPause<true>(sessions)},
            {"sessions.resume", porla::Methods::SessionsResume<true>(sessions)},
            {"sessions.settings.list", porla::Methods::SessionsSettingsList<true>(sessions)},
            {"sys.versions", porla::Methods::SysVersions<true>()},
            {"torrents.add", porla::Methods::TorrentsAdd<true>(sessions, cfg->presets)},
            {"torrents.files.list", porla::Methods::TorrentsFilesList<true>(sessions)},
            {"torrents.list", porla::Methods::TorrentsList<true>(sessions)},
            {"torrents.metadata.list", porla::Methods::TorrentsMetadataList<true>(cfg->db, sessions)},
            {"torrents.move", porla::Methods::TorrentsMove<true>(sessions)},
            {"torrents.pause", porla::Methods::TorrentsPause<true>(sessions)},
            {"torrents.peers.add", porla::Methods::TorrentsPeersAdd<true>(sessions)},
            {"torrents.peers.list", porla::Methods::TorrentsPeersList<true>(sessions)},
            {"torrents.properties.get", porla::Methods::TorrentsPropertiesGet<true>(sessions)},
            {"torrents.properties.set", porla::Methods::TorrentsPropertiesSet<true>(sessions)},
            {"torrents.recheck", porla::Methods::TorrentsRecheck<true>(sessions)},
            {"torrents.remove", porla::Methods::TorrentsRemove<true>(sessions)},
            {"torrents.resume", porla::Methods::TorrentsResume<true>(sessions)},
            {"torrents.trackers.list", porla::Methods::TorrentsTrackersList<true>(sessions)}
        });

        porla::Http::JsonRpcHandler<false> rpc({
            {"fs.space", porla::Methods::FsSpace<false>()},
            {"plugins.configure", porla::Methods::PluginsConfigure<false>(plugin_engine)},
            {"plugins.get", porla::Methods::PluginsGet<false>(plugin_engine)},
            {"plugins.install", porla::Methods::PluginsInstall<false>(plugins_install_options)},
            {"plugins.list", porla::Methods::PluginsList<false>(plugin_engine)},
            {"plugins.reload", porla::Methods::PluginsReload<false>(plugin_engine)},
            {"plugins.uninstall", porla::Methods::PluginsUninstall<false>(plugin_engine)},
            {"plugins.update", porla::Methods::PluginsUpdate<false>(plugins_update_options)},
            {"presets.list", porla::Methods::PresetsList<false>(cfg->presets)},
            {"sessions.list", porla::Methods::SessionsList<false>(sessions)},
            {"sessions.pause", porla::Methods::SessionsPause<false>(sessions)},
            {"sessions.resume", porla::Methods::SessionsResume<false>(sessions)},
            {"sessions.settings.list", porla::Methods::SessionsSettingsList<false>(sessions)},
            {"sys.versions", porla::Methods::SysVersions<false>()},
            {"torrents.add", porla::Methods::TorrentsAdd<false>(sessions, cfg->presets)},
            {"torrents.files.list", porla::Methods::TorrentsFilesList<false>(sessions)},
            {"torrents.list", porla::Methods::TorrentsList<false>(sessions)},
            {"torrents.metadata.list", porla::Methods::TorrentsMetadataList<false>(cfg->db, sessions)},
            {"torrents.move", porla::Methods::TorrentsMove<false>(sessions)},
            {"torrents.pause", porla::Methods::TorrentsPause<false>(sessions)},
            {"torrents.peers.add", porla::Methods::TorrentsPeersAdd<false>(sessions)},
            {"torrents.peers.list", porla::Methods::TorrentsPeersList<false>(sessions)},
            {"torrents.properties.get", porla::Methods::TorrentsPropertiesGet<false>(sessions)},
            {"torrents.properties.set", porla::Methods::TorrentsPropertiesSet<false>(sessions)},
            {"torrents.recheck", porla::Methods::TorrentsRecheck<false>(sessions)},
            {"torrents.remove", porla::Methods::TorrentsRemove<false>(sessions)},
            {"torrents.resume", porla::Methods::TorrentsResume<false>(sessions)},
            {"torrents.trackers.list", porla::Methods::TorrentsTrackersList<false>(sessions)}
        });

        std::string http_base_path = cfg->http_base_path.value_or("/");
        if (http_base_path.empty())        http_base_path = "/";
        if (http_base_path[0] != '/')      http_base_path = "/" + http_base_path;
        if (http_base_path.ends_with("/")) http_base_path = http_base_path.substr(0, http_base_path.size() - 1);

        if (cfg->ssl_enable.has_value() && cfg->ssl_enable.value())
        {
            uWS::Loop::get(&io);
            
            BOOST_LOG_TRIVIAL(info) << "Starting HTTP server with SSL ON.";

            uWS::SSLApp http_server({.key_file_name = cfg->ssl_key_file.value().c_str(),
                                     .cert_file_name = cfg->ssl_cert_file.value().c_str(),
                                     .passphrase = cfg->ssl_key_file_pass.value().c_str()});

            http_server.post(
                http_base_path + "/api/v1/auth/init",
                porla::Http::AuthInitHandler(io, cfg->db, cfg->sodium_memlimit.value_or(crypto_pwhash_MEMLIMIT_MIN)));
            http_server.post(http_base_path + "/api/v1/auth/login",
                             porla::Http::AuthLoginHandler(porla::Http::AuthLoginHandlerOptions{
                                 .db = cfg->db, .io = io, .secret_key = cfg->secret_key}));

            http_server.get(http_base_path + "/api/v1/events",
                            cfg->http_auth_enabled.value_or(true)
                                ? static_cast<porla::Http::Handler<true>>(
                                      porla::Http::JwtHandler<true>(cfg->secret_key, porla::Http::EventsHandler<true>(sessions)))
                                : static_cast<porla::Http::Handler<true>>(porla::Http::EventsHandler<true>(sessions)));

            http_server.post(http_base_path + "/api/v1/jsonrpc",
                             cfg->http_auth_enabled.value_or(true)
                                 ? static_cast<porla::Http::Handler<true>>(porla::Http::JwtHandler<true>(cfg->secret_key, rpcSSL))
                                 : static_cast<porla::Http::Handler<true>>(rpcSSL));

            http_server.get(http_base_path + "/api/v1/system", porla::Http::SystemHandler<true>(cfg->db));

            if (cfg->http_metrics_enabled.value_or(true))
            {
                BOOST_LOG_TRIVIAL(info) << "Enabling HTTP metrics endpoint";
                http_server.get(http_base_path + "/metrics", porla::Http::MetricsHandler<true>(sessions));
            }

            if (cfg->http_webui_enabled.value_or(true))
            {
                BOOST_LOG_TRIVIAL(info) << "Enabling HTTP web UI";
                http_server.get(http_base_path + "/*", porla::Http::WebUIHandler<true>(http_base_path));
            }

            http_server.listen(cfg->http_host.value_or("127.0.0.1"), cfg->http_port.value_or(1337),
                               [](const auto *t) { BOOST_LOG_TRIVIAL(info) << "HTTP server listening"; });
            
            uWS::run();
        }
        else
        {
            uWS::Loop::get(&io);

            BOOST_LOG_TRIVIAL(info) << "Starting HTTP server with SSL OFF.";
            
            uWS::App http_server;

            http_server.post(
                http_base_path + "/api/v1/auth/init",
                porla::Http::AuthInitHandler(io, cfg->db, cfg->sodium_memlimit.value_or(crypto_pwhash_MEMLIMIT_MIN)));
            http_server.post(http_base_path + "/api/v1/auth/login",
                             porla::Http::AuthLoginHandler(porla::Http::AuthLoginHandlerOptions{
                                 .db = cfg->db, .io = io, .secret_key = cfg->secret_key}));

            http_server.get(http_base_path + "/api/v1/events",
                            cfg->http_auth_enabled.value_or(true)
                                ? static_cast<porla::Http::Handler<false>>(
                                      porla::Http::JwtHandler<false>(cfg->secret_key, porla::Http::EventsHandler<false>(sessions)))
                                : static_cast<porla::Http::Handler<false>>(porla::Http::EventsHandler<false>(sessions)));

            http_server.post(http_base_path + "/api/v1/jsonrpc",
                             cfg->http_auth_enabled.value_or(true)
                                 ? static_cast<porla::Http::Handler<false>>(porla::Http::JwtHandler<false>(cfg->secret_key, rpc))
                                 : static_cast<porla::Http::Handler<false>>(rpc));

            http_server.get(http_base_path + "/api/v1/system", porla::Http::SystemHandler<false>(cfg->db));

            if (cfg->http_metrics_enabled.value_or(true))
            {
                BOOST_LOG_TRIVIAL(info) << "Enabling HTTP metrics endpoint";
                http_server.get(http_base_path + "/metrics", porla::Http::MetricsHandler<false>(sessions));
            }

            if (cfg->http_webui_enabled.value_or(true))
            {
                BOOST_LOG_TRIVIAL(info) << "Enabling HTTP web UI";
                http_server.get(http_base_path + "/*", porla::Http::WebUIHandler<false>(http_base_path));
            }

            http_server.listen(cfg->http_host.value_or("127.0.0.1"), cfg->http_port.value_or(1337),
                               [](const auto *t) { BOOST_LOG_TRIVIAL(info) << "HTTP server listening"; });

            uWS::run();
        }

        // io.run() breaks SSLApp

        plugin_engine.UnloadAll();
    }

    git_libgit2_shutdown();
    curl_global_cleanup();

    return 0;
}
