#include "authenticator.hpp"

#include <boost/algorithm/string.hpp>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>

#include "../utils/string.hpp"

using porla::Auth::Authenticator;
using porla::Auth::Context;
using porla::Utils::String;

namespace
{
    const std::string AltAuthHeader = "x-porla-token";

    const auto CookieFinder = [](const std::string_view& value) -> std::optional<std::string>
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

        for (const auto& item : values)
        {
            std::vector<std::string> pair = String::Split(item, "=");

            if (pair.size() != 2)
            {
                continue;
            }

            if (boost::trim_copy(pair[0]) == "porla-auth-token")
            {
                return pair[1];
            }
        }

        return std::nullopt;
    };

    const auto HeaderFinder = [](uWS::HttpRequest* req, const std::string& header_name) -> std::optional<std::string>
    {
        const auto header = req->getHeader(header_name);

        if (header.empty()) { return std::nullopt; }

        // Authorization carries "Bearer <token>"; the alt header carries the token itself.
        if (header.size() > 7 && boost::iequals(header.substr(0, 7), "bearer "))
        {
            return std::string(header.substr(7));
        }

        return std::string(header);
    };

}

Authenticator::Authenticator(sqlite3* db, std::string secret_key)
    : m_db(db)
    , m_secret_key(std::move(secret_key))
{
}

Context Authenticator::Authenticate(uWS::HttpRequest* req) const
{
    std::optional<std::string> bearer_token = HeaderFinder(req, AltAuthHeader);

    // No alt header found, or the alt header didn't contain a value. Check the default Authorization header
    if (!bearer_token.has_value())
    {
        bearer_token = HeaderFinder(req, "authorization");
    }

    if (!bearer_token.has_value())
    {
        bearer_token = CookieFinder(req->getHeader("cookie"));
    }

    // We could not find a bearer token at all. No need to create any kind
    // of context more than the default.
    if (!bearer_token)
    {
        return {};
    }

    if (bearer_token->size() >= 2
        && (*bearer_token)[0] == 'e'
        && (*bearer_token)[1] == 'y')
    {
        return DecodeJwt(bearer_token.value());
    }

    return {};
}

Context Authenticator::DecodeJwt(const std::string& encoded_token) const
{
    const auto algo   = jwt::algorithm::hs256(m_secret_key);
    const auto issuer = "porla";

    const auto verifier = jwt::verify()
        .allow_algorithm(algo)
        .with_issuer(issuer);

    try
    {
        const auto decoded_token = jwt::decode(encoded_token);

        verifier.verify(decoded_token);

        return Context{
            .kind    = Context::Kind::User,
            .subject = decoded_token.get_subject()
        };
    }
    catch (...)
    {
        return {};
    }
}
