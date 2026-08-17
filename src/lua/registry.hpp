#pragma once

#include <boost/asio.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>
#include <uWebSockets/App.h>

#include "../sessions.hpp"

namespace porla::Lua::Registry
{
    struct Ops
    {
        std::map<std::size_t, sol::protected_function>                    callbacks;
        std::map<std::size_t, std::shared_ptr<boost::asio::steady_timer>> steady_timers;
        std::size_t                                                       next_id = 1;
    };

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
