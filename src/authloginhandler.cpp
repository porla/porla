#include "authloginhandler.hpp"

#include <boost/log/trivial.hpp>
#include <jwt-cpp/jwt.h>
#include <sodium.h>

#include "data/models/users.hpp"

using porla::AuthLoginHandler;
using porla::Data::Models::Users;

AuthLoginHandler::AuthLoginHandler(boost::asio::io_context& io, const AuthLoginHandlerOptions& opts)
    : m_io(io)
    , m_db(opts.db)
    , m_secret_key(opts.secret_key)
{
}

void AuthLoginHandler::operator()(const std::shared_ptr<HttpContext>& ctx)
{
    if (m_threads.size() >= 5)
    {
        return ctx->Write("Noo");
    }

    const auto req = nlohmann::json::parse(ctx->Request().body());

    if (!req.contains("username") || !req.contains("password"))
    {
        return ctx->Write("Invalid request");
    }

    auto const username = req["username"].get<std::string>();
    auto const password = req["password"].get<std::string>();

    auto const user = Users::GetByUsername(m_db, username);

    m_threads.emplace_back(
        [ctx, &io = m_io, password, secret_key = m_secret_key, &threads = m_threads, user]()
        {
            int result = -1;

            if (user)
            {
                result = crypto_pwhash_str_verify(
                    user->password_hashed.c_str(),
                    password.c_str(),
                    password.size());
            }
            else
            {
                // No user found. Still calculate a pwhash to not make it easy to
                // enumerate usernames. Do not set the result. The hashed password is
                // just 'hunter2'.

                (void) crypto_pwhash_str_verify(
                    "$argon2id$v=19$m=1048576,t=4,p=1$MNy6/sqw4+WlGyeRDxiFdw$+FnYmB7Qfz+JKQeCzpjQW7rmpW/uqZxwGqDRDweBQRE",
                    "hunter2",
                    7);
            }

            boost::asio::dispatch(
                io,
                [ctx, result, secret_key, thread_id = std::this_thread::get_id(), &threads, username = user->username]()
                {
                    auto thread = std::find_if(
                        threads.begin(),
                        threads.end(),
                        [&thread_id](auto const& t) { return t.get_id() == thread_id; });

                    if (thread == threads.end())
                    {
                        // This really shouldn't happen.
                        BOOST_LOG_TRIVIAL(error) << "Could not find thread";
                    }
                    else
                    {
                        BOOST_LOG_TRIVIAL(debug) << "Erasing worker thread " << thread_id;

                        if (thread->joinable())
                        {
                            thread->join();
                        }

                        threads.erase(thread);
                    }

                    if (result != 0)
                    {
                        return ctx->WriteJson({
                            {"error", "invalid_auth"}
                        });
                    }

                    // Issue a JWT valid for 1 day. This should be enough for most users.
                    auto token = jwt::create()
                        .set_expires_at(std::chrono::system_clock::now() + std::chrono::days{1})
                        .set_issuer("porla")
                        .set_issued_at(std::chrono::system_clock::now())
                        .set_subject(username)
                        .set_type("JWS")
                        .sign(jwt::algorithm::hs256(secret_key));

                    ctx->WriteJson({
                        {"token", token}
                    });
                });
        });
}
