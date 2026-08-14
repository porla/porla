#include "jsonrpc.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#include "../utils/string.hpp"

using porla::Rpc::JsonRpc;
using porla::Utils::String;

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

class DefaultResponseWriter : public porla::Rpc::ResponseWriter
{
public:
    explicit DefaultResponseWriter(uWS::HttpResponse<false>* res, const nlohmann::json& id)
        : m_id(id)
        , m_res(res)
    {
    }

    void Error(int code, const std::string& message, const nlohmann::json& data = {}) override
    {
        m_res->end(json({
            {"jsonrpc", "2.0"},
            {"id", m_id},
            {"error", {
                {"code", code},
                {"message", message},
                {"data", data}
            }}
        }).dump());
    }

    void Header(const std::string& key, const std::string& value) override
    {
        m_res->writeHeader(key, value);
    }

    void Ok(const nlohmann::json& result) override
    {
        m_res->end(json({
            {"jsonrpc", "2.0"},
            {"id", m_id},
            {"result", result}
        }).dump());
    }

private:
    nlohmann::json            m_id;
    uWS::HttpResponse<false>* m_res;
};

JsonRpc::JsonRpc(const std::string& secret_key)
    : m_secret_key(secret_key)
{
}

std::shared_ptr<JsonRpc> JsonRpc::Create(const std::string& secret_key)
{
    return std::shared_ptr<JsonRpc>(new JsonRpc(secret_key));
}

std::function<void(uWS::HttpResponse<false>*, uWS::HttpRequest*)> JsonRpc::HttpHandler()
{
    return [weak = weak_from_this()](uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
    {
        auto jsonrpc = weak.lock();

        if (jsonrpc == nullptr)
        {
            res->writeStatus("503 Unavailable")
                ->end("JSONRPC server gone to sleep");

            return;
        }

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

        auto buffer = std::make_shared<std::string>();

        res->onAborted([]{});
        res->onData([buffer, res, weak, bearer_token = std::move(bearer_token)](std::string_view data, bool last)
        {
            buffer->append(data);
            if (!last) return;

            auto jsonrpc = weak.lock();

            if (jsonrpc == nullptr)
            {
                res->writeStatus("503 Unavailable")
                    ->end("JSONRPC server gone to sleep");

                return;
            }

            Token token;

            if (bearer_token)
            {
                try
                {
                    const auto decoded_token = jwt::decode(bearer_token.value());

                    const auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::hs256(jsonrpc->m_secret_key))
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
                body = json::parse(*buffer);
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

            std::string method_name = body.at("method").get<std::string>();

            if (jsonrpc->m_methods.find(method_name) == jsonrpc->m_methods.end())
            {
                BOOST_LOG_TRIVIAL(debug) << "Failed to find JSONRPC method '" << method_name << "'";

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
                BOOST_LOG_TRIVIAL(debug) << "Executing JSONRPC method '" << method_name << "'";

                json params = body.contains("params")
                    ? body.at("params")
                    : json();

                auto method = jsonrpc->m_methods.at(method_name);

                auto writer = std::make_shared<DefaultResponseWriter>(res, body["id"]);

                if (!method->CanInvoke(token))
                {
                    writer->Error(1001, "Invocation not allowed");
                    return;
                }

                method->Invoke(params, writer);
            }
            catch (const std::exception& ex)
            {
                BOOST_LOG_TRIVIAL(error) << "Error when executing JSONRPC method '" << method_name << "': " << ex.what();

                res->end(json({
                    {"error", {
                        {"code", -32603},
                        {"message", "Internal error"},
                        {"data", ex.what()}
                    }}
                }).dump());
            }
        });
    };
}
