#pragma once

#include <sqlite3.h>

#include "authlogin_reqres.hpp"
#include "../../typedmethod.hpp"

namespace porla::Rpc::Methods::Auth
{
    class AuthLogin : public TypedMethod<AuthLoginReq, AuthLoginRes>
    {
    public:
        explicit AuthLogin(sqlite3* db, const std::string& secret_key);

    protected:
        bool CanInvoke(Token token) override
        {
            return true;
        }

        void Execute(const AuthLoginReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        std::string m_secret_key;
    };
}
