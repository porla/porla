#include "httpjwtauth.hpp"

#include <utility>

#include <jwt-cpp/jwt.h>

using porla::HttpJwtAuth;

HttpJwtAuth::HttpJwtAuth(std::string secret_key, std::optional<std::string> alt_header, porla::HttpMiddleware middleware)
    : m_secret_key(std::move(secret_key))
    , m_alt_header(alt_header)
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

    const auto header_finder = [&ctx]<typename T>(const T& header)
    {
        auto const auth_header = ctx->Request().find(header);

        // No Authorization header
        if (auth_header == ctx->Request().end())
        {
            return std::optional<std::string>();
        }

        // Authorization header is too short to start with "Bearer " and also contain a token.
        if (auth_header->value().size() <= 7)
        {
            return std::optional<std::string>();
        }

        return std::optional<std::string>(auth_header->value().substr(7));
    };

    std::optional<std::string> bearer_token;

    // Check the alt header for a token, if an alt header is specified
    if (m_alt_header.has_value())
    {
        bearer_token = header_finder(m_alt_header.value());
    }

    // No alt header found, or the alt header didn't contain a value
    if (!bearer_token.has_value())
    {
        bearer_token = header_finder(http::field::authorization);
    }

    if (!bearer_token.has_value())
    {
        return ctx->Write(not_authorized());
    }

    try
    {
        auto decoded_token = jwt::decode(bearer_token.value());

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
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(warning) << "Failed to decode token: " << ex.what();
    }

    return ctx->Write(not_authorized());
}
