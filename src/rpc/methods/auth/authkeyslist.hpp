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
        void Execute(const AuthKeysListReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
    };
}
