#include <boost/asio.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <sqlite3.h>

#include "config.hpp"
#include "httpeventstream.hpp"
#include "httpserver.hpp"
#include "jsonrpchandler.hpp"
#include "session.hpp"

#include "data/migrate.hpp"
#include "methods/sessionpause.hpp"
#include "methods/sessionsettingslist.hpp"
#include "methods/torrentsadd.hpp"
#include "methods/torrentsfileslist.hpp"
#include "methods/torrentsget.hpp"
#include "methods/torrentslist.hpp"
#include "methods/torrentsmove.hpp"
#include "methods/torrentspause.hpp"
#include "methods/torrentspeersadd.hpp"
#include "methods/torrentspeerslist.hpp"
#include "methods/torrentsquery.hpp"
#include "methods/torrentsremove.hpp"
#include "methods/torrentsresume.hpp"
#include "methods/torrentstrackerslist.hpp"

int PrintSettings(const libtorrent::settings_pack& settings)
{
    for (int i = lt::settings_pack::bool_type_base; i < lt::settings_pack::max_bool_setting_internal; i++)
    {
        if (strcmp(lt::name_for_setting(i), "") == 0) continue;
        if (settings.has_val(i))
            printf("%s = %s\n", lt::name_for_setting(i), settings.get_bool(i) ? "true" : "false");
    }

    for (int i = lt::settings_pack::int_type_base; i < lt::settings_pack::max_int_setting_internal; i++)
    {
        if (strcmp(lt::name_for_setting(i), "") == 0) continue;
        if (settings.has_val(i))
            printf("%s = %d\n", lt::name_for_setting(i), settings.get_int(i));
    }

    for (int i = lt::settings_pack::string_type_base; i < lt::settings_pack::max_string_setting_internal; i++)
    {
        if (strcmp(lt::name_for_setting(i), "") == 0) continue;
        if (settings.has_val(i) && lt::name_for_setting(i) != nullptr)
            printf("%s = \"%s\"\n", lt::name_for_setting(i), settings.get_str(i).c_str());
    }

    return 0;
}

int main(int argc, char* argv[])
{
    porla::Config cfg = porla::Config::Load(argc, argv);

    // Set up some debugging commands
    if (argc >= 2 && strcmp(argv[1], "debug:settings") == 0)
    {
        return PrintSettings(cfg.session_settings);
    }

    boost::log::trivial::severity_level log_level = boost::log::trivial::info;
    if (cfg.log_level == "trace")   { log_level = boost::log::trivial::severity_level::trace; }
    if (cfg.log_level == "debug")   { log_level = boost::log::trivial::severity_level::debug; }
    if (cfg.log_level == "info")    { log_level = boost::log::trivial::severity_level::info; }
    if (cfg.log_level == "warning") { log_level = boost::log::trivial::severity_level::warning; }
    if (cfg.log_level == "error")   { log_level = boost::log::trivial::severity_level::error; }
    if (cfg.log_level == "fatal")   { log_level = boost::log::trivial::severity_level::fatal; }
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= log_level);

    boost::asio::io_context io;
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);

    boost::asio::deadline_timer check_pid_timer(io);
    std::function<void(boost::system::error_code)> check_pid;

    if (cfg.supervised_pid)
    {
        BOOST_LOG_TRIVIAL(info) << "Running in supervised mode - parent: " << cfg.supervised_pid.value();

        check_pid = [&cfg, &check_pid, &check_pid_timer, &io](boost::system::error_code ec)
        {
            if (ec)
            {
                return;
            }

            if (kill(cfg.supervised_pid.value(), 0) != 0)
            {
                BOOST_LOG_TRIVIAL(warning) << "Parent process died. Shutting down.";
                io.stop();
                return;
            }

            check_pid_timer.expires_from_now(boost::posix_time::seconds(cfg.supervised_interval.value_or(1)));
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

    sqlite3* db;
    sqlite3_open(cfg.db.value_or("porla.sqlite").c_str(), &db);
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    if (!porla::Data::Migrate(db))
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to run migrations";
        return -1;
    }

    {
        porla::Session session(io, porla::SessionOptions{
            .db                    = db,
            .settings              = cfg.session_settings,
            .timer_dht_stats       = cfg.timer_dht_stats.value_or(5000),
            .timer_session_stats   = cfg.timer_session_stats.value_or(5000),
            .timer_torrent_updates = cfg.timer_torrent_updates.value_or(1000)
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
            {"session.pause", porla::Methods::SessionPause(session)},
            {"session.settings.list", porla::Methods::SessionSettingsList(session)},
            {"torrents.add", porla::Methods::TorrentsAdd(session)},
            {"torrents.files.list", porla::Methods::TorrentsFilesList(session)},
            {"torrents.get", porla::Methods::TorrentsGet(session)},
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
            .host = cfg.http_host.value_or("127.0.0.1"),
            .port = cfg.http_port.value_or(1337)
        });

        http.Use(porla::HttpPost("/api/v1/jsonrpc", [&rpc](auto const& ctx) { rpc(ctx); }));
        http.Use(porla::HttpGet("/api/v1/events", porla::HttpEventStream(session)));
        http.Use(porla::HttpNotFound());

        io.run();
    }

    BOOST_LOG_TRIVIAL(info) << "Vacuuming database";

    sqlite3_exec(db, "VACUUM;", nullptr, nullptr, nullptr);
    sqlite3_close(db);

    return 0;
}
