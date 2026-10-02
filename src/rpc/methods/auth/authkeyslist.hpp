#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "authkeyslist_reqres.hpp"

namespace porla::Rpc::Methods::Auth
{
    class AuthKeysList : public TypedMethod<AuthKeysListReq, AuthKeysListRes>
    {
    public:
        explicit AuthKeysList(sqlite3* db);

    protected:
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return auth_ctx.IsAuthenticated() && auth_ctx.kind == porla::Auth::Context::Kind::User;
        }

        void Execute(const AuthKeysListReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
    };
}
