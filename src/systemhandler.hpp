#pragma once

#include <sqlite3.h>

#include "httpcontext.hpp"

namespace porla
{
    class SystemHandler
    {
    public:
        explicit SystemHandler(sqlite3* db);
        void operator()(const std::shared_ptr<HttpContext>&);

    private:
        sqlite3* m_db;
    };
}