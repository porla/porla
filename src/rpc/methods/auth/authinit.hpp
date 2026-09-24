#pragma once

#include <boost/asio/io_context.hpp>
#include <sqlite3.h>

#include "../../typedasyncmethod.hpp"

#include "authinit_reqres.hpp"

namespace porla::Rpc::Methods::Auth
{
    class AuthInit : public TypedAsyncMethod<AuthInitReq, AuthInitRes>
    {
    public:
        explicit AuthInit(boost::asio::io_context& io, sqlite3* db);

    protected:
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return true;
        }

        boost::asio::awaitable<void> ExecuteAsync(AuthInitReq req, ResponseWriterHandle writer) override;

    private:
        sqlite3* m_db;
    };
}
