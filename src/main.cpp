#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <sqlite3.h>

#include "httpmiddleware.hpp"
#include "httpserver.hpp"
#include "session.hpp"

#include "data/migrate.hpp"
#include "methods/torrentsadd.hpp"
#include "methods/torrentslist.hpp"
#include "methods/torrentsquery.hpp"

int main(int argc, char* argv[])
{
    boost::asio::io_context io;
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);

    signals.async_wait(
        [&io](boost::system::error_code const& ec, int signal)
        {
            BOOST_LOG_TRIVIAL(info) << "Interrupt received (" << signal << ") - stopping...";
            io.stop();
        });

    sqlite3* db;
    sqlite3_open("porla.sqlite", &db);
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    if (!porla::Data::Migrate(db))
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to run migrations";
        return -1;
    }

    porla::Session session(io, porla::SessionOptions{
        .db = db
    });

    try
    {
        session.Load();
    }
    catch(const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load torrents: " << ex.what();
        return -1;
    }

    porla::HttpServer http(io, porla::HttpServerOptions{
        .host = "127.0.0.1",
        .port = 1337
    });

    http.Use(porla::Methods::TorrentsAdd("/api/torrents.add", session));
    http.Use(porla::Methods::TorrentsList("/api/torrents.list", session));
    http.Use(porla::Methods::TorrentsQuery("/api/torrents.query", session));

    io.run();

    sqlite3_close(db);

    return 0;
}
