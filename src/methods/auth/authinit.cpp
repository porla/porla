#include "authinit.hpp"

#include <boost/log/trivial.hpp>
#include <sodium.h>

#include "../../data/models/users.hpp"

using porla::Methods::Auth::AuthInit;
using porla::Methods::Auth::AuthInitReq;
using porla::Methods::Auth::AuthInitRes;

AuthInit::AuthInit(sqlite3* db)
    : m_db(db)
{
}

void AuthInit::Invoke(const AuthInitReq& req, WriteCb<AuthInitRes> cb)
{
    if (porla::Data::Models::Users::Any(m_db))
    {
        return cb.Error(-1, "Already initialized");
    }


    std::string password_hashed;
    password_hashed.resize(crypto_pwhash_STRBYTES);

    int result = crypto_pwhash_str(
        password_hashed.data(),
        req.password.c_str(),
        req.password.size(),
        crypto_pwhash_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_MEMLIMIT_MIN);

    porla::Data::Models::Users::Insert(
        m_db,
        porla::Data::Models::Users::User{
            .username        = req.username,
            .password_hashed = password_hashed,
        });

    BOOST_LOG_TRIVIAL(info) << "User " << req.username << " created";

    cb.Ok(AuthInitRes{});
}
