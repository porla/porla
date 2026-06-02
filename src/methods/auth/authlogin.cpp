#include "authlogin.hpp"

#include <boost/log/trivial.hpp>
#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <jwt-cpp/jwt.h>
#include <sodium.h>

#include "../../data/models/users.hpp"

using porla::Data::Models::Users;
using porla::Methods::Auth::AuthLogin;
using porla::Methods::Auth::AuthLoginReq;
using porla::Methods::Auth::AuthLoginRes;

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

AuthLogin::AuthLogin(sqlite3* db, const std::string& secret_key)
    : m_db(db)
    , m_secret_key(secret_key)
{
}

void AuthLogin::Invoke(const AuthLoginReq& req, WriteCb<AuthLoginRes> cb)
{
    const auto user = Users::GetByUsername(m_db, req.username);

    int result = -1;

    if (user)
    {
        result = crypto_pwhash_str_verify(
            user->password_hashed.c_str(),
            req.password.c_str(),
            req.password.size());
    }
    else
    {
        // No user found. Still calculate a pwhash to not make it easy to
        // enumerate usernames. Do not set the result. The hashed password is
        // just 'hunter2'.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
        (void) crypto_pwhash_str_verify(
            "$argon2id$v=19$m=8,t=2,p=1$+6Dhs+XwJA6aMr+EpEirUA$5+DoNO4hoWzPUnTM0BTg3a0JZx0c9LI1FkfZvDX1lTw",
            "hunter2",
            7);
#pragma GCC diagnostic pop
    }

    if (result != 0 || !user.has_value())
    {
        return cb.Error(-1, "Invalid username/password combination");
    }

    const auto token = jwt::create()
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::days{1})
        .set_issuer("porla")
        .set_issued_at(std::chrono::system_clock::now())
        .set_subject(user->username)
        .set_type("JWS")
        .set_payload_claim("scope", "*")
        .sign(jwt::algorithm::hs256(m_secret_key));

    cb.Header("Set-Cookie", CreateAuthCookie("porla-auth-token", token, 86400, false));

    cb.Ok(AuthLoginRes{
        .token = token
    });
}
