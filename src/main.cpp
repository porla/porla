#include <boost/asio.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

#include "buildinfo.hpp"
#include "config.hpp"
#include "httpauthtokenhandler.hpp"
#include "httpeventstream.hpp"
#include "httpserver.hpp"
#include "jsonrpchandler.hpp"
#include "metricshandler.hpp"
#include "session.hpp"

#include "methods/presetslist.hpp"
#include "methods/sessionpause.hpp"
#include "methods/sessionresume.hpp"
#include "methods/sessionsettingslist.hpp"
#include "methods/sessionsettingsupdate.hpp"
#include "methods/sysversions.hpp"
#include "methods/torrentsadd.hpp"
#include "methods/torrentsfileslist.hpp"
#include "methods/torrentslist.hpp"
#include "methods/torrentsmove.hpp"
#include "methods/torrentspause.hpp"
#include "methods/torrentspeersadd.hpp"
#include "methods/torrentspeerslist.hpp"
#include "methods/torrentsquery.hpp"
#include "methods/torrentsremove.hpp"
#include "methods/torrentsresume.hpp"
#include "methods/torrentstrackerslist.hpp"

int PrintJsonVersion()
{
    printf("{\"branch\": \"%s\",\"commitish\": \"%s\", \"version\": \"%s\"}\n",
        porla::BuildInfo::Branch(),
        porla::BuildInfo::Commitish(),
        porla::BuildInfo::Version());

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc >= 2 && strcmp(argv[1], "version:json") == 0)
    {
        return PrintJsonVersion();
    }

    std::unique_ptr<porla::Config> cfg = nullptr;

    try
    {
        cfg = porla::Config::Load(argc, argv);
    }
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to load configuration: " << ex.what();
        return -1;
    }

    boost::log::trivial::severity_level log_level = boost::log::trivial::info;
    if (cfg->log_level == "trace")   { log_level = boost::log::trivial::severity_level::trace; }
    if (cfg->log_level == "debug")   { log_level = boost::log::trivial::severity_level::debug; }
    if (cfg->log_level == "info")    { log_level = boost::log::trivial::severity_level::info; }
    if (cfg->log_level == "warning") { log_level = boost::log::trivial::severity_level::warning; }
    if (cfg->log_level == "error")   { log_level = boost::log::trivial::severity_level::error; }
    if (cfg->log_level == "fatal")   { log_level = boost::log::trivial::severity_level::fatal; }
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= log_level);

    boost::asio::io_context io;
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);

    boost::asio::deadline_timer check_pid_timer(io);
    std::function<void(boost::system::error_code)> check_pid;

    if (cfg->supervised_pid)
    {
        BOOST_LOG_TRIVIAL(info) << "Running in supervised mode - parent: " << cfg->supervised_pid.value();

        check_pid = [&cfg, &check_pid, &check_pid_timer, &io](boost::system::error_code ec)
        {
            if (ec)
            {
                return;
            }

            if (kill(cfg->supervised_pid.value(), 0) != 0)
            {
                BOOST_LOG_TRIVIAL(warning) << "Parent process died. Shutting down.";
                io.stop();
                return;
            }

            check_pid_timer.expires_from_now(boost::posix_time::seconds(cfg->supervised_interval.value_or(1)));
            check_pid_timer.async_wait(check_pid);
        };

        check_pid(boost::system::error_code());
    }

    signals.async_wait(
        [&io](boost::system::error_code const& ec, int signal)
        {
            BOOST_LOG_TRIVIAL(info) << "Interrupt received (" << signal << ") - stopping...";
            io.stop();
        });

    {
        porla::Session session(io, porla::SessionOptions{
            .db                    = cfg->db,
            .extensions            = cfg->session_extensions,
            .settings              = cfg->session_settings,
            .timer_dht_stats       = cfg->timer_dht_stats.value_or(5000),
            .timer_session_stats   = cfg->timer_session_stats.value_or(5000),
            .timer_torrent_updates = cfg->timer_torrent_updates.value_or(1000)
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

        porla::JsonRpcHandler rpc({
            {"presets.list", porla::Methods::PresetsList(cfg->presets)},
            {"session.pause", porla::Methods::SessionPause(session)},
            {"session.resume", porla::Methods::SessionResume(session)},
            {"session.settings.list", porla::Methods::SessionSettingsList(session)},
            {"session.settings.update", porla::Methods::SessionSettingsUpdate(session, cfg->db)},
            {"sys.versions", porla::Methods::SysVersions()},
            {"torrents.add", porla::Methods::TorrentsAdd(session, cfg->presets)},
            {"torrents.files.list", porla::Methods::TorrentsFilesList(session)},
            {"torrents.list", porla::Methods::TorrentsList(session)},
            {"torrents.move", porla::Methods::TorrentsMove(session)},
            {"torrents.pause", porla::Methods::TorrentsPause(session)},
            {"torrents.peers.add", porla::Methods::TorrentsPeersAdd(session)},
            {"torrents.peers.list", porla::Methods::TorrentsPeersList(session)},
            {"torrents.query", porla::Methods::TorrentsQuery(session)},
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

        if (cfg->http_auth_token)
        {
            http.Use(porla::HttpAuthTokenHandler(cfg->http_auth_token.value()));
        }

        http.Use(porla::HttpPost("/api/v1/jsonrpc", [&rpc](auto const& ctx) { rpc(ctx); }));
        http.Use(porla::HttpGet("/api/v1/events", [&eventStream](auto const& ctx) { eventStream(ctx); }));
        http.Use(porla::HttpGet("/metrics", [&metrics](auto const& ctx) { metrics(ctx); }));
        http.Use(porla::HttpNotFound());

        // If we run in supervised mode - print connection information here and finish
        // with a 'ready' message to allow the supervisor to connect.
        if (cfg->supervised_pid)
        {
            printf(":: set http_port=%d\n", http.Endpoint().port());
            printf(":: ready\n");
            fflush(stdout);
        }

        io.run();
    }

    return 0;
}
