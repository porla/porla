#pragma once

#include <boost/asio/any_io_executor.hpp>
#include <sqlite3.h>

#include "authlogin_reqres.hpp"
#include "../../typedasyncmethod.hpp"

namespace porla::Rpc::Methods::Auth
{
    class AuthLogin : public TypedAsyncMethod<AuthLoginReq, AuthLoginRes>
    {
    public:
        explicit AuthLogin(boost::asio::io_context& io, sqlite3* db, const std::string& secret_key);

    protected:
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return true;
        }

        boost::asio::awaitable<void> ExecuteAsync(AuthLoginReq req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        std::string m_secret_key;
    };
}
