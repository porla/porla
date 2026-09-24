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
        Authenticator(sqlite3* db, std::string secret_key);

        Context Authenticate(uWS::HttpRequest* req) const;

    private:
        Context DecodeJwt(const std::string& encoded_token) const;

        sqlite3*    m_db;
        std::string m_secret_key;
    };
}
