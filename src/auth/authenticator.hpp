#pragma once

#include <string>

#include <sqlite3.h>
#include <uWebSockets/App.h>

#include "context.hpp"

namespace porla::Auth
{
    class Authenticator
    {
    public:
        Authenticator(sqlite3* db);

        Context Authenticate(uWS::HttpRequest* req) const;

    private:
        sqlite3*    m_db;
    };
}
