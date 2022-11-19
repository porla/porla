#include "httpjwtauth.hpp"

#include <utility>

#include <jwt-cpp/jwt.h>

using porla::HttpJwtAuth;

HttpJwtAuth::HttpJwtAuth(std::string secret_key, porla::HttpMiddleware middleware)
    : m_secret_key(std::move(secret_key))
    , m_http_middleware(std::move(middleware))
{
}

void HttpJwtAuth::operator()(const std::shared_ptr<porla::HttpContext> &ctx)
{
    namespace http = boost::beast::http;

    auto const not_authorized = [&ctx]()
    {
        http::response<http::string_body> res{http::status::unauthorized, ctx->Request().version()};
        res.set(http::field::server, "porla/1.0");
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(ctx->Request().keep_alive());
        res.body() = "Unauthorized";
        res.prepare_payload();
        return res;
    };

    auto const auth_header = ctx->Request().find(http::field::authorization);

    // No Authorization header
    if (auth_header == ctx->Request().end())
    {
        return ctx->Write(not_authorized());
    }

    // Authorization header is too short to start with "Bearer " and also contain a token.
    if (auth_header->value().size() <= 7)
    {
        return ctx->Write(not_authorized());
    }

    const std::string bearer_token = auth_header->value()
        .substr(7)
        .to_string();

    try
    {
        auto decoded_token = jwt::decode(bearer_token);

        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256(m_secret_key))
            .with_issuer("porla");

        verifier.verify(decoded_token);

        return m_http_middleware(ctx);
    }
    catch (const jwt::signature_verification_exception& ex)
    {
        BOOST_LOG_TRIVIAL(warning) << "Failed to verify JWT signature: " << ex.what();
    }
    catch (const jwt::token_verification_exception& ex)
    {
        BOOST_LOG_TRIVIAL(warning) << "Failed to verify JWT token: " << ex.what();
    }

    return ctx->Write(not_authorized());
}
