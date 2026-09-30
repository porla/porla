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
        void Execute(const AuthKeysCreateReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
    };
}
