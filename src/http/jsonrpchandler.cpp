#include "jsonrpchandler.hpp"

#include <boost/algorithm/string.hpp>
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

JsonRpcHandler::JsonRpcHandler(const std::string& secret_key, std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<false>*, std::optional<jwt::decoded_jwt<jwt::traits::nlohmann_json>>)>> methods)
    : m_secret_key(secret_key)
    , m_state(std::make_shared<State>(methods))
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

        if (boost::trim_copy(pair[0]) == "porla-auth-token")
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

    std::string buffer;
    res->onData([req, res, bearer_token = std::move(bearer_token), secret_key = m_secret_key, state = m_state, buffer = std::move(buffer)](std::string_view d, bool last) mutable
    {
        buffer.append(d.data(), d.length());
        if (!last) return;

        res->writeStatus("200 OK")
            ->writeHeader("Content-Type", "application/json");

        std::optional<jwt::decoded_jwt<jwt::traits::nlohmann_json>> token;

        if (bearer_token)
        {
            try
            {
                const auto decoded_token = jwt::decode(bearer_token.value());

                const auto verifier = jwt::verify()
                    .allow_algorithm(jwt::algorithm::hs256(secret_key))
                    .with_issuer("porla");

                verifier.verify(decoded_token);

                token = decoded_token;
            }
            catch (const jwt::error::signature_verification_exception& ex)
            {
                BOOST_LOG_TRIVIAL(debug) << "Failed to verify JWT signature: " << ex.what();

                res->end(json({
                    {"error", {
                        {"code", 1000},
                        {"message", "Invalid JWT signature"},
                        {"data", {
                            {"what", ex.what()}
                        }}
                    }}
                }).dump());

                return;
            }
            catch (const jwt::error::token_verification_exception& ex)
            {
                BOOST_LOG_TRIVIAL(debug) << "Failed to verify JWT token: " << ex.what();

                res->end(json({
                    {"error", {
                        {"code", 1000},
                        {"message", "JWT verification failed"},
                        {"data", {
                            {"what", ex.what()}
                        }}
                    }}
                }).dump());

                return;
            }
            catch (const std::exception& ex)
            {
                BOOST_LOG_TRIVIAL(debug) << "Failed to decode token: " << ex.what();

                res->end(json({
                    {"error", {
                        {"code", 1000},
                        {"message", "Failed to decode JWT"},
                        {"data", {
                            {"what", ex.what()}
                        }}
                    }}
                }).dump());

                return;
            }
        }

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

        if (!body.contains("method")
            && !body["method"].is_string())
        {
            res->end(json({
                {"error", {
                    {"code", -32600},
                    {"message", "Invalid Request"},
                    {"data", "Method is not a string"}
                }}
            }).dump());

            return;
        }

        std::string method = body.at("method").get<std::string>();
        auto methods = state->Methods();

        if (methods.find(method) == methods.end())
        {
            BOOST_LOG_TRIVIAL(debug) << "Failed to find JSONRPC method '" << method << "'";

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

            json params = body.contains("params")
                ? body.at("params")
                : json();

            methods.at(method)(body.at("id"), params, res, token);
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
