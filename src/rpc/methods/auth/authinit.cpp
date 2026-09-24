#include "authinit.hpp"

#include <boost/log/trivial.hpp>

#include "../../../auth/password.hpp"
#include "../../../data/models/users.hpp"

using porla::Auth::Password;
using porla::Rpc::Methods::Auth::AuthInit;
using porla::Rpc::Methods::Auth::AuthInitReq;
using porla::Rpc::Methods::Auth::AuthInitRes;

AuthInit::AuthInit(boost::asio::io_context& io, boost::asio::thread_pool& hash_pool, sqlite3* db)
    : TypedAsyncMethod(io.get_executor())
    , m_hash_pool(hash_pool)
    , m_db(db)
{
}

boost::asio::awaitable<void> AuthInit::ExecuteAsync(AuthInitReq req, ResponseWriterHandle out)
{
    if (porla::Data::Models::Users::Any(m_db))
    {
        co_return out->Error(-1, "Already initialized");
    }

    const auto hashed_password = co_await Password::Hash(m_hash_pool, std::move(req.password));

    if (!hashed_password.has_value())
    {
        co_return out->Error(-2, "Failed to hash password");
    }

    if (porla::Data::Models::Users::Any(m_db))
    {
        BOOST_LOG_TRIVIAL(warning) << "An initial user was created while hashing password";
        co_return out->Error(-1, "Already initialized");
    }

    porla::Data::Models::Users::Insert(
        m_db,
        porla::Data::Models::Users::User{
            .username        = req.username,
            .password_hashed = hashed_password.value()
        });

    BOOST_LOG_TRIVIAL(info) << "User " << req.username << " created";

    out->Ok(AuthInitRes{});
}
