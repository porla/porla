#pragma once

#include <boost/asio.hpp>
#include <sqlite3.h>

#include "handler.hpp"

namespace porla::Http
{
    class AuthInitHandler
    {
    public:
        explicit AuthInitHandler(boost::asio::io_context& io, sqlite3* db, int memlimit);

        void operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req);

    private:
        boost::asio::io_context& m_io;
        sqlite3* m_db;
        int m_memlimit;
    };
}
