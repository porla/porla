#pragma once

#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <sqlite3.h>

#include "httpcontext.hpp"

namespace porla
{
    class AuthLoginHandler
    {
    public:
        explicit AuthLoginHandler(boost::asio::io_context& io, sqlite3* db);
        ~AuthLoginHandler();

        void operator()(const std::shared_ptr<HttpContext>&);

    private:
        boost::asio::io_context& m_io;
        sqlite3* m_db;
        std::vector<std::thread> m_workers;
    };
}
