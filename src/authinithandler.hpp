#pragma once

#include <boost/asio.hpp>
#include <sqlite3.h>

#include "httpcontext.hpp"

namespace porla
{
    class AuthInitHandler
    {
    public:
        explicit AuthInitHandler(boost::asio::io_context& io, sqlite3* db, int memlimit);

        void operator()(const std::shared_ptr<HttpContext>&);

    private:
        boost::asio::io_context& m_io;
        sqlite3* m_db;
        int m_memlimit;
    };
}
