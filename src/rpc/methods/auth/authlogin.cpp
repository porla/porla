#include "authlogin.hpp"

#include <boost/log/trivial.hpp>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <jwt-cpp/jwt.h>

#include <sodium.h>

#include "../../../auth/password.hpp"
#include "../../../data/models/users.hpp"

using porla::Auth::Password;
using porla::Data::Models::Users;
using porla::Rpc::Methods::Auth::AuthLogin;
using porla::Rpc::Methods::Auth::AuthLoginReq;
using porla::Rpc::Methods::Auth::AuthLoginRes;

static std::string CreateAuthCookie(const std::string& name, const std::string& token, int max_age_seconds, bool secure)
{
    std::stringstream ss;
    ss << name << "=" << token << "; Path=/; HttpOnly; Max-Age=" << max_age_seconds << "; SameSite=Strict";

    if (secure)
    {
        ss << "; Secure";
    }

    return ss.str();
}

AuthLogin::AuthLogin(boost::asio::io_context& io, boost::asio::thread_pool& hash_pool, sqlite3* db, const std::string& secret_key)
    : TypedAsyncMethod(io.get_executor())
    , m_hash_pool(hash_pool)
    , m_db(db)
    , m_secret_key(secret_key)
{
}

boost::asio::awaitable<void> AuthLogin::ExecuteAsync(AuthLoginReq req, ResponseWriterHandle cb)
{
    const auto user = Users::GetByUsername(m_db, req.username);

    const auto& stored = user.has_value()
        ? user->password_hashed
        : "$argon2id$v=19$m=65536,t=2,p=1$Po8JYTODUsewD1r3zg1XsQ$icZxLXBcZRDPKybU+9MkOIdrATTmhENNG7EfckW5zvg";

    const auto [ verified, needs_rehash ] = co_await Password::Verify(m_hash_pool, stored, req.password);

    if (!verified || !user.has_value())
    {
        cb->Error(-1, "Invalid username/password combination");
        co_return;
    }

    if (needs_rehash)
    {
        const auto fresh_password = co_await Password::Hash(m_hash_pool, req.password);

        if (fresh_password)
        {
            Data::Models::Users::UpdatePassword(
                m_db,
                user->id,
                fresh_password.value());

            BOOST_LOG_TRIVIAL(info) << "Updated password hash for user " << user->username;
        }
    }

    const auto token = jwt::create()
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::days{1})
        .set_issuer("porla")
        .set_issued_at(std::chrono::system_clock::now())
        .set_subject(user->username)
        .set_type("JWS")
        .set_payload_claim("scope", "*")
        .sign(jwt::algorithm::hs256(m_secret_key));

    cb->Header("Set-Cookie", CreateAuthCookie("porla-auth-token", token, 86400, false));

    cb->Ok(AuthLoginRes{
        .token = token
    });
}
