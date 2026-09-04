#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "authinit_reqres.hpp"

namespace porla::Rpc::Methods::Auth
{
    class AuthInit : public TypedMethod<AuthInitReq, AuthInitRes>
    {
    public:
        explicit AuthInit(sqlite3* db);

    protected:
        bool CanInvoke(Token token) override
        {
            return true;
        }

        void Execute(const AuthInitReq& req, ResponseWriterHandle writer) override;

    private:
        sqlite3* m_db;
    };
}
