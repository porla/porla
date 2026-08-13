#include "jsonrpc.hpp"

#include <boost/log/trivial.hpp>

using porla::Rpc::JsonRpc;

class DefaultResponseWriter : public porla::Rpc::ResponseWriter
{
public:
    explicit DefaultResponseWriter(uWS::HttpResponse<false>* res)
    {
    }

    void Error(int code, const std::string& message, const nlohmann::json& data = {}) override
    {
    }

    void Header(const std::string& name, const std::string& value) override
    {
    }

    void Ok(const nlohmann::json& j) override
    {
    }

    void Write(const nlohmann::json& j) override
    {
    }
};

std::shared_ptr<JsonRpc> JsonRpc::Create()
{
    return std::make_shared<JsonRpc>();
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

        auto buffer = std::make_shared<std::string>();

        res->onData([buffer, res, weak](std::string_view data, bool last)
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

                if (!method->CanInvoke({}))
                {
                    return;
                }

                method->Invoke(params, std::make_unique<DefaultResponseWriter>(res));
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
