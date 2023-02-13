#include <boost/asio.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

#include "authinithandler.hpp"
#include "authloginhandler.hpp"
#include "cmdargs.hpp"
#include "config.hpp"
#include "embeddedwebuihandler.hpp"
#include "httpeventstream.hpp"
#include "httpjwtauth.hpp"
#include "httpserver.hpp"
#include "jsonrpchandler.hpp"
#include "logger.hpp"
#include "metricshandler.hpp"
#include "session.hpp"
#include "systemhandler.hpp"
#include "tools/authtoken.hpp"
#include "tools/generatesecretkey.hpp"
#include "tools/mediainfoparse.hpp"
#include "tools/versionjson.hpp"
#include "utils/secretkey.hpp"

#include "methods/fsspace.hpp"
#include "methods/presetslist.hpp"
#include "methods/sessionpause.hpp"
#include "methods/sessionresume.hpp"
#include "methods/sessionsettingslist.hpp"
#include "methods/sessionsettingsupdate.hpp"
#include "methods/sysversions.hpp"
#include "methods/torrentsadd.hpp"
#include "methods/torrentsfileslist.hpp"
#include "methods/torrentslist.hpp"
#include "methods/torrentsmediainfo.hpp"
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

#include "workflows/actionfactory.hpp"
#include "workflows/executor.hpp"
#include "workflows/workflow.hpp"
#include "workflows/actions/http.hpp"
#include "workflows/actions/log.hpp"
#include "workflows/actions/sleep.hpp"
#include "workflows/actions/push/discord.hpp"
#include "workflows/actions/push/ntfy.hpp"
#include "workflows/actions/torrents/move.hpp"
#include "workflows/actions/torrents/reannounce.hpp"
#include "workflows/actions/torrents/remove.hpp"

int main(int argc, char* argv[])
{
    static std::map<std::string, std::function<int(int, char**, std::unique_ptr<porla::Config>)>> subcommands =
    {
        {"auth:token", &porla::Tools::AuthToken},
        {"key:generate", &porla::Tools::GenerateSecretKey},
        {"mediainfo:parse", &porla::Tools::MediaInfoParse},
        {"version:json", &porla::Tools::VersionJson}
    };

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
            .mediainfo_enabled          = cfg->mediainfo_enabled.value_or(false),
            .mediainfo_file_extensions  = cfg->mediainfo_file_extensions,
            .mediainfo_file_min_size    = cfg->mediainfo_file_min_size.value_or(100 * 1024 * 1024),
            .mediainfo_file_wanted_size = cfg->mediainfo_file_wanted_size.value_or(1 * 1024 * 1024),
            .settings                   = cfg->session_settings,
            .session_params_file        = cfg->state_dir.value_or(fs::current_path()) / "session.dat",
            .timer_dht_stats            = cfg->timer_dht_stats.value_or(5000),
            .timer_session_stats        = cfg->timer_session_stats.value_or(5000),
            .timer_torrent_updates      = cfg->timer_torrent_updates.value_or(1000)
        });

        try
        {
            session.Load();
        }
        catch (const std::exception &ex)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to load torrents: " << ex.what();
            return -1;
        }

        std::vector<std::shared_ptr<porla::Workflows::Workflow>> workflows;

        BOOST_LOG_TRIVIAL(info) << "Loading " << cfg->workflow_files.size() << " workflow file(s)";

        for (const auto& workflow_file : cfg->workflow_files)
        {
            BOOST_LOG_TRIVIAL(debug) << "Loading workflow from file " << workflow_file;
            workflows.push_back(porla::Workflows::Workflow::LoadFromFile(workflow_file));
        }

        porla::Workflows::Executor workflow_executor{porla::Workflows::ExecutorOptions{
            .session        = session,
            .workflows      = workflows,
            .action_factory = std::make_shared<porla::Workflows::ActionFactory>(
                std::map<std::string, std::function<std::shared_ptr<porla::Workflows::Action>()>>{
                    // The world is not ready for this {"http",                [&io]()      { return std::make_shared<porla::Workflows::Actions::Http>(io); }},
                    {"log",                 []()         { return std::make_shared<porla::Workflows::Actions::Log>(); }},
                    {"push/discord",        [&io]()      { return std::make_shared<porla::Workflows::Actions::Push::Discord>(io); }},
                    {"push/ntfy-sh",        [&io]()      { return std::make_shared<porla::Workflows::Actions::Push::Ntfy>(io); }},
                    {"sleep",               [&io]()      { return std::make_shared<porla::Workflows::Actions::Sleep>(io); }},
                    {"torrents/move",       [&session]() { return std::make_shared<porla::Workflows::Actions::Torrents::Move>(session); }},
                    {"torrents/reannounce", [&session]() { return std::make_shared<porla::Workflows::Actions::Torrents::Reannounce>(session); }},
                    {"torrents/remove",     [&session]() { return std::make_shared<porla::Workflows::Actions::Torrents::Remove>(session); }}
                })
        }};

        porla::JsonRpcHandler rpc({
            {"fs.space", porla::Methods::FsSpace()},
            {"presets.list", porla::Methods::PresetsList(cfg->presets)},
            {"session.pause", porla::Methods::SessionPause(session)},
            {"session.resume", porla::Methods::SessionResume(session)},
            {"session.settings.list", porla::Methods::SessionSettingsList(session)},
            {"session.settings.update", porla::Methods::SessionSettingsUpdate(session, cfg->db)},
            {"sys.versions", porla::Methods::SysVersions()},
            {"torrents.add", porla::Methods::TorrentsAdd(cfg->db, session, cfg->presets)},
            {"torrents.files.list", porla::Methods::TorrentsFilesList(session)},
            {"torrents.list", porla::Methods::TorrentsList(cfg->db, session)},
            {"torrents.mediainfo", porla::Methods::TorrentsMediaInfo(session)},
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

        porla::HttpServer http(io, porla::HttpServerOptions{
            .host = cfg->http_host.value_or("127.0.0.1"),
            .port = cfg->http_port.value_or(1337)
        });

        porla::HttpEventStream eventStream(session);
        porla::MetricsHandler metrics(session);

        porla::AuthInitHandler authInitHandler(io, cfg->db);
        porla::AuthLoginHandler authLoginHandler(io, porla::AuthLoginHandlerOptions{
            .db         = cfg->db,
            .secret_key = cfg->secret_key
        });

        std::string http_base_path = cfg->http_base_path.value_or("/");
        if (http_base_path.empty())        http_base_path = "/";
        if (http_base_path[0] != '/')      http_base_path = "/" + http_base_path;
        if (http_base_path.ends_with("/")) http_base_path = http_base_path.substr(0, http_base_path.size() - 1);

        http.Use(porla::HttpPost(http_base_path + "/api/v1/auth/init",  [&authInitHandler](auto const& ctx) { authInitHandler(ctx); }));
        http.Use(porla::HttpPost(http_base_path + "/api/v1/auth/login", [&authLoginHandler](auto const& ctx) { authLoginHandler(ctx); }));
        http.Use(porla::HttpGet(http_base_path +  "/api/v1/system",     porla::SystemHandler(cfg->db)));

        http.Use(
            porla::HttpPost(http_base_path + "/api/v1/jsonrpc",
                cfg->http_auth_enabled.value_or(true)
                    ? static_cast<porla::HttpMiddleware>(porla::HttpJwtAuth(cfg->secret_key, cfg->http_alt_auth_header, [&rpc](auto const& ctx) { rpc(ctx); }))
                    : static_cast<porla::HttpMiddleware>([&rpc](auto const& ctx) { rpc(ctx); })));

        http.Use(
            porla::HttpGet(http_base_path + "/api/v1/events",
                cfg->http_auth_enabled.value_or(true)
                    ? static_cast<porla::HttpMiddleware>(porla::HttpJwtAuth(cfg->secret_key, cfg->http_alt_auth_header, [&eventStream](auto const& ctx) { eventStream(ctx); }))
                    : static_cast<porla::HttpMiddleware>([&eventStream](auto const& ctx) { eventStream(ctx); })));

        if (cfg->http_metrics_enabled.value_or(true))
        {
            BOOST_LOG_TRIVIAL(info) << "Enabling HTTP metrics endpoint";
            http.Use(porla::HttpGet(http_base_path + "/metrics", [&metrics](auto const &ctx) { metrics(ctx); }));
        }

        if (cfg->http_webui_enabled.value_or(true))
        {
            BOOST_LOG_TRIVIAL(info) << "Enabling HTTP web UI";
            http.Use(porla::EmbeddedWebUIHandler(http_base_path));
        }

        http.Use(porla::HttpNotFound());

        io.run();
    }

    return 0;
}
