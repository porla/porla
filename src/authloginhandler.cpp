#include "authloginhandler.hpp"

#include <jwt-cpp/jwt.h>
#include <sodium.h>

#include "data/models/users.hpp"

using porla::AuthLoginHandler;

AuthLoginHandler::AuthLoginHandler(boost::asio::io_context& io, const AuthLoginHandlerOptions& opts)
    : m_io(io)
    , m_db(opts.db)
    , m_secret_key(opts.secret_key)
{
}

void AuthLoginHandler::operator()(const std::shared_ptr<HttpContext>& ctx)
{
    const auto req = nlohmann::json::parse(ctx->Request().body());

    if (!req.contains("username") || !req.contains("password"))
    {
        return ctx->Write("Invalid request");
    }

    auto const username = req["username"].get<std::string>();
    auto const password = req["password"].get<std::string>();

    auto const user = porla::Data::Models::Users::GetByUsername(m_db, username);

    if (!user)
    {
        return ctx->Write("No");
    }

    std::thread t(
        [ctx, &io = m_io, password, secret_key = m_secret_key, user]()
        {
            int result = crypto_pwhash_str_verify(
                user->password_hashed.c_str(),
                password.c_str(),
                password.size());

            boost::asio::dispatch(
                io,
                [ctx, result, secret_key, username = user->username]()
                {
                    if (result != 0)
                    {
                        return ctx->WriteJson({
                            {"error", "invalid_auth"}
                        });
                    }

                    auto token = jwt::create()
                        .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{3600})
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

    t.detach();
}