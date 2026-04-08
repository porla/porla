#pragma once

#include <boost/asio/io_context.hpp>
#include <sqlite3.h>

namespace porla::Lua::Registry
{
    struct BoostIoContext
    {
        boost::asio::io_context* io;
    };

    struct Sqlite3
    {
        sqlite3* db;
    };
}
