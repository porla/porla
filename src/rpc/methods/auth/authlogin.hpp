#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/thread_pool.hpp>
#include <sqlite3.h>

#include "authlogin_reqres.hpp"
#include "../../typedasyncmethod.hpp"

namespace porla::Rpc::Methods::Auth
{
    class AuthLogin : public TypedAsyncMethod<AuthLoginReq, AuthLoginRes>
    {
    public:
        explicit AuthLogin(boost::asio::io_context& io, boost::asio::thread_pool& hash_pool, sqlite3* db, const std::string& secret_key);

    protected:
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return true;
        }

        boost::asio::awaitable<void> ExecuteAsync(AuthLoginReq req, ResponseWriterHandle cb) override;

    private:
        boost::asio::thread_pool& m_hash_pool;
        sqlite3* m_db;
        std::string m_secret_key;
    };
}
