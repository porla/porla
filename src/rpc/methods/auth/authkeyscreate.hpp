#pragma once

#include <sqlite3.h>

#include "authkeyscreate_reqres.hpp"
#include "../../typedmethod.hpp"

namespace porla::Rpc::Methods::Auth
{
    class AuthKeysCreate : public TypedMethod<AuthKeysCreateReq, AuthKeysCreateRes>
    {
    public:
        explicit AuthKeysCreate(sqlite3* db);

    protected:
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return auth_ctx.IsAuthenticated() && auth_ctx.kind == porla::Auth::Context::Kind::User;
        }

        void Execute(const AuthKeysCreateReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
    };
}
