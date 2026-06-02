#include "jsonrpchandler.hpp"

#include <boost/log/trivial.hpp>

#include "../utils/string.hpp"

using json = nlohmann::json;

using porla::Http::JsonRpcHandler;
using porla::Utils::String;

class JsonRpcHandler::State
{
public:
    explicit State(std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<false>*, std::optional<jwt::decoded_jwt<jwt::traits::nlohmann_json>>)>> methods)
        : m_methods(std::move(methods))
    {
    }

    std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<false>*, std::optional<jwt::decoded_jwt<jwt::traits::nlohmann_json>>)>>& Methods()
    {
        return m_methods;
    }

private:
    std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<false>*, std::optional<jwt::decoded_jwt<jwt::traits::nlohmann_json>>)>> m_methods;
};

JsonRpcHandler::JsonRpcHandler(std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<false>*, std::optional<jwt::decoded_jwt<jwt::traits::nlohmann_json>>)>> methods)
    : m_state(std::make_shared<State>(methods))
{
}


static const std::string AltAuthHeader = "x-porla-token";

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

        if (pair[0] == "porla-auth-token")
        {
            return pair[1];
        }
    }

    return std::nullopt;
};

const auto HeaderFinder = [](uWS::HttpRequest* req, const std::string& header_name)
{
    const auto& header = req->getHeader(header_name);

    // No Authorization header
    if (header.empty())
    {
        return std::optional<std::string>();
    }

    // Authorization header is too short to start with "Bearer " and also contain a token.
    if (header.size() <= 7)
    {
        return std::optional<std::string>();
    }

    return std::optional<std::string>(header.substr(7));
};

void JsonRpcHandler::operator()(
    uWS::HttpResponse<false>* res,
    uWS::HttpRequest* req)
{
    res->onAborted([](){});

    std::string buffer;
    res->onData([req, res, state = m_state, buffer = std::move(buffer)](std::string_view d, bool last) mutable
    {
        buffer.append(d.data(), d.length());
        if (!last) return;

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

        std::optional<jwt::decoded_jwt<jwt::traits::nlohmann_json>> token;

        if (bearer_token)
        {
            try
            {
                token = jwt::decode(bearer_token.value());

                const auto verifier = jwt::verify()
                    .allow_algorithm(jwt::algorithm::hs256("m_secret_key"))
                    .with_issuer("porla");

                verifier.verify(token.value());
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
        }

        res->writeStatus("200 OK")
            ->writeHeader("Content-Type", "application/json");

        json body = {};

        try
        {
            body = json::parse(buffer);
        }
        catch (const std::exception& ex)
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to parse JSONRPC request: " << ex.what();

            json error = {
                {"error", {
                    {"code", -32700},
                    {"message", "Parse error"},
                    {"data", ex.what()}
                }}
            };

            res->end(error.dump());

            return;
        }

        if (!body.contains("id")
            && !body["id"].is_string()
            && !body["id"].is_number()
            && !body["id"].is_null())
        {
            res->end(json({
                {"error", {
                    {"code", -32600},
                    {"message", "Invalid Request"},
                    {"data", "Id is not a string, number or null"}
                }}
            }).dump());

            return;
        }

        std::string method = body.at("method").get<std::string>();

        if (state->Methods().find(method) == state->Methods().end())
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to find JSONRPC method '" << method << "'";

            res->end(json({
                {"error", {
                    {"code", -32601},
                    {"message", "Method not found"}
                }}
            }).dump());

            return;
        }

        try
        {
            BOOST_LOG_TRIVIAL(debug) << "Executing JSONRPC method '" << method << "'";
            state->Methods().at(method)(body.at("id"), body.at("params"), res, token);
        }
        catch (const std::exception& ex)
        {
            BOOST_LOG_TRIVIAL(error) << "Error when executing JSONRPC method '" << method << "': " << ex.what();

            res->end(json({
                {"error", {
                    {"code", -32603},
                    {"message", "Internal error"},
                    {"data", ex.what()}
                }}
            }).dump());
        }
    });
}
