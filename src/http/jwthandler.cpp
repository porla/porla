#include "jwthandler.hpp"

#include <boost/log/trivial.hpp>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <jwt-cpp/jwt.h>

#include "../utils/string.hpp"

using porla::Http::JwtHandler;
using porla::Utils::String;

JwtHandler::JwtHandler(const std::string &secret_key, Handler next)
    : m_secret_key(secret_key)
    , m_next(next)
{
}

void JwtHandler::operator()(uWS::HttpResponse<false> *res, uWS::HttpRequest *req)
{
    static const std::string AltAuthHeader = "x-porla-token";

    const auto cookie_finder = [](const std::string_view& value) -> std::optional<std::string>
    {
        if (value.empty())
        {
            return std::nullopt;
        }

        std::vector<std::string> values = String::Split(std::string(value), ";");

        if (values.empty())
        {
            return std::nullopt;
        }

        for (const auto& cookie_item : values)
        {
            std::vector<std::string> pair = String::Split(cookie_item, "=");

            if (pair.size() != 2)
            {
                continue;
            }

            if (pair[0] == "porla-auth-token")
            {
                return pair[1];
            }
        }

        return std::nullopt;
    };

    const auto header_finder = [&req](const std::string& header)
    {
        const auto& auth_header = req->getHeader(header);

        // No Authorization header
        if (auth_header.empty())
        {
            return std::optional<std::string>();
        }

        // Authorization header is too short to start with "Bearer " and also contain a token.
        if (auth_header.size() <= 7)
        {
            return std::optional<std::string>();
        }

        return std::optional<std::string>(auth_header.substr(7));
    };

    std::optional<std::string> bearer_token = header_finder(AltAuthHeader);

    // No alt header found, or the alt header didn't contain a value. Check the default Authorization header
    if (!bearer_token.has_value())
    {
        bearer_token = header_finder("authorization");
    }

    if (!bearer_token.has_value())
    {
        bearer_token = cookie_finder(req->getHeader("cookie"));
    }

    if (!bearer_token.has_value())
    {
        return res->writeStatus("401 Unauthorized")->end("Unauthorized");
    }

    try
    {
        const auto decoded_token = jwt::decode(bearer_token.value());

        const auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256(m_secret_key))
            .with_issuer("porla");

        verifier.verify(decoded_token);

        return m_next(res, req);
    }
    catch (const jwt::error::signature_verification_exception& ex)
    {
        BOOST_LOG_TRIVIAL(warning) << "Failed to verify JWT signature: " << ex.what();
    }
    catch (const jwt::error::token_verification_exception& ex)
    {
        BOOST_LOG_TRIVIAL(warning) << "Failed to verify JWT token: " << ex.what();
    }
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(warning) << "Failed to decode token: " << ex.what();
    }

    return res->writeStatus("401 Unauthorized")->end("Unauthorized");
}
