#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "authlogin_reqres.hpp"

namespace porla::Methods::Auth
{
    class AuthLogin : public Method<AuthLoginReq, AuthLoginRes>
    {
    public:
        explicit AuthLogin(sqlite3* db, const std::string& secret_key);

    protected:
        void Invoke(const AuthLoginReq& req, WriteCb<AuthLoginRes> cb) override;

    private:
        sqlite3* m_db;
        std::string m_secret_key;
    };
}
