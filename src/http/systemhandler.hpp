#pragma once

#include <sqlite3.h>

#include "handler.hpp"

namespace porla::Http
{
    class SystemHandler
    {
    public:
        explicit SystemHandler(sqlite3* db);

        void operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req);

    private:
        sqlite3* m_db;
    };
}
