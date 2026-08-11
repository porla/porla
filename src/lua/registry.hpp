#pragma once

#include <boost/asio.hpp>
#include <sqlite3.h>
#include <uWebSockets/App.h>

#include "../sessions.hpp"

namespace porla::Lua::Registry
{
    struct BoostIoContext
    {
        boost::asio::io_context* io;
    };

    struct uWebSocketsApp
    {
        uWS::App* app;
    };

    struct Sessions
    {
        porla::Sessions& sessions;
    };

    struct Sqlite3
    {
        sqlite3* db;
    };
}
