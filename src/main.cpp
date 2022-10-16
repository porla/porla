#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <sqlite3.h>

#include "config.hpp"
#include "httpeventstream.hpp"
#include "httpserver.hpp"
#include "jsonrpchandler.hpp"
#include "session.hpp"
#include "settingspack.hpp"

#include "data/migrate.hpp"
#include "methods/torrentsadd.hpp"
#include "methods/torrentsget.hpp"
#include "methods/torrentspeersadd.hpp"
#include "methods/torrentsquery.hpp"
#include "methods/torrentsremove.hpp"

int PrintSettings(const toml::table& cfg)
{
    auto settings = porla::SettingsPack::Load(cfg);

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
    toml::table cfg = porla::Config::Load(argc, argv);

    // Set up some debugging commands
    if (argc >= 2 && strcmp(argv[1], "debug:settings") == 0)
    {
        return PrintSettings(cfg);
    }

    boost::asio::io_context io;
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);

    signals.async_wait(
        [&io](boost::system::error_code const& ec, int signal)
        {
            BOOST_LOG_TRIVIAL(info) << "Interrupt received (" << signal << ") - stopping...";
            io.stop();
        });

    sqlite3* db;
    sqlite3_open(cfg["sqlite"]["file"].value_or("porla.sqlite"), &db);
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    if (!porla::Data::Migrate(db))
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to run migrations";
        return -1;
    }

    {
        porla::Session session(io, porla::SessionOptions{
            .db = db,
            .settings = porla::SettingsPack::Load(cfg),
            .timer_dht_stats = cfg["timers"]["dht_stats"].value_or(5000),
            .timer_session_stats = cfg["timers"]["session_stats"].value_or(5000),
            .timer_torrent_updates = cfg["timers"]["torrent_updates"].value_or(1000)
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
            {"torrents.add", porla::Methods::TorrentsAdd(session)},
            {"torrents.get", porla::Methods::TorrentsGet(session)},
            {"torrents.peers.add", porla::Methods::TorrentsPeersAdd(session)},
            {"torrents.query", porla::Methods::TorrentsQuery(session)},
            {"torrents.remove", porla::Methods::TorrentsRemove(session)}
        });

        porla::HttpServer http(io, porla::HttpServerOptions{
            .host = cfg["http"]["host"].value_or("127.0.0.1"),
            .port = cfg["http"]["port"].value_or<uint16_t>(1337)
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
