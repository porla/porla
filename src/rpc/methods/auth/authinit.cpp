#include "authinit.hpp"

#include <boost/log/trivial.hpp>
#include <sodium.h>

#include "../../../data/models/users.hpp"

using porla::Rpc::Methods::Auth::AuthInit;
using porla::Rpc::Methods::Auth::AuthInitReq;
using porla::Rpc::Methods::Auth::AuthInitRes;

AuthInit::AuthInit(boost::asio::io_context& io, sqlite3* db)
    : TypedAsyncMethod(io.get_executor())
    , m_db(db)
{
}

boost::asio::awaitable<void> AuthInit::ExecuteAsync(AuthInitReq req, ResponseWriterHandle out)
{
    if (porla::Data::Models::Users::Any(m_db))
    {
        out->Error(-1, "Already initialized");
        co_return;
    }

    std::string password_hashed;
    password_hashed.resize(crypto_pwhash_STRBYTES);

    int result = crypto_pwhash_str(
        password_hashed.data(),
        req.password.c_str(),
        req.password.size(),
        crypto_pwhash_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_MEMLIMIT_INTERACTIVE);

    porla::Data::Models::Users::Insert(
        m_db,
        porla::Data::Models::Users::User{
            .username        = req.username,
            .password_hashed = password_hashed,
        });

    BOOST_LOG_TRIVIAL(info) << "User " << req.username << " created";

    out->Ok(AuthInitRes{});
}
