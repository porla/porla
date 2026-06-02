#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "authinit_reqres.hpp"

namespace porla::Methods::Auth
{
    class AuthInit : public Method<AuthInitReq, AuthInitRes>
    {
    public:
        explicit AuthInit(sqlite3* db);

    protected:
        bool CanInvoke(Token token) override
        {
            return true;
        }

        void Invoke(const AuthInitReq& req, WriteCb<AuthInitRes> cb) override;

    private:
        sqlite3* m_db;
    };
}
