#include "authinithandler.hpp"

#include <thread>

#include <boost/log/trivial.hpp>
#include <sodium.h>

#include "data/models/users.hpp"

using porla::AuthInitHandler;

AuthInitHandler::AuthInitHandler(boost::asio::io_context& io, sqlite3* db, int memlimit)
    : m_io(io)
    , m_db(db)
    , m_memlimit(memlimit)
{
}

void AuthInitHandler::operator()(const std::shared_ptr<HttpContext>& ctx)
{
    if (porla::Data::Models::Users::Any(m_db))
    {
        return ctx->Write("Invalid request");
    }

    const auto req = nlohmann::json::parse(ctx->Request().body());

    if (!req.contains("username") || !req.contains("password"))
    {
        return ctx->Write("Invalid request");
    }

    auto const username = req["username"].get<std::string>();
    auto const password = req["password"].get<std::string>();

    std::thread t(
        [ctx = ctx, db = m_db, &io = m_io, mem = m_memlimit, username, password]()
        {
            std::string password_hashed;
            password_hashed.resize(crypto_pwhash_STRBYTES);

            int result = crypto_pwhash_str(
                password_hashed.data(),
                password.c_str(),
                password.size(),
                crypto_pwhash_OPSLIMIT_SENSITIVE,
                mem);

            boost::asio::dispatch(
                io,
                [ctx, db, password_hashed, result, username]()
                {
                    if (result != 0)
                    {
                        BOOST_LOG_TRIVIAL(error) << "Out of memory when hashing password";

                        return ctx->WriteJson({
                            {"error", "oom"}
                        });
                    }

                    if (porla::Data::Models::Users::Any(db))
                    {
                        BOOST_LOG_TRIVIAL(warning) << "A user was created while we where creating ours";

                        return ctx->WriteJson({
                            {"error", "already_initialized"}
                        });
                    }

                    porla::Data::Models::Users::Insert(
                        db,
                        porla::Data::Models::Users::User{
                            .username        = username,
                            .password_hashed = password_hashed,
                        });

                    BOOST_LOG_TRIVIAL(info) << "User " << username << " created";

                    ctx->WriteJson({
                        {"ok", true}
                    });
                });
        });

    t.detach();
}
