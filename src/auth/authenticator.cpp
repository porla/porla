#include "authenticator.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>
#include <sodium.h>

#include "tokens.hpp"

#include "../utils/string.hpp"

using porla::Auth::Authenticator;
using porla::Auth::Context;
using porla::Utils::String;

namespace Tokens = porla::Auth::Tokens;

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

Authenticator::Authenticator(sqlite3* db)
    : m_db(db)
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

    std::optional<porla::Data::Models::Tokens::Token> token;

    try
    {
        token = Tokens::Verify(m_db, bearer_token.value());
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to verify token: " << e.what();
    }

    if (!token.has_value())
    {
        return {};
    }

    if (token->kind == "session")
    {
        // session token from a user that has been removed
        if (!token->username.has_value())
        {
            return {};
        }

        return Context{
            .kind     = Context::Kind::User,
            .subject  = token->username,
            .token_id = token->id
        };
    }

    return Context{
        .kind     = Context::Kind::Machine,
        .subject  = token->id,
        .token_id = token->id
    };
}
