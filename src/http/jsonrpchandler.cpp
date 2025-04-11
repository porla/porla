#include "jsonrpchandler.hpp"

#include <boost/log/trivial.hpp>

using json = nlohmann::json;
using porla::Http::JsonRpcHandler;

template <bool SSL> template <bool SSLS> class JsonRpcHandler<SSL>::State
{
public:
    explicit State(std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<SSLS>*)>> methods)
        : m_methods(std::move(methods))
    {
    }

    std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<SSLS>*)>>& Methods()
    {
        return m_methods;
    }

private:
    std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<SSLS>*)>> m_methods;
};

template <bool SSL> JsonRpcHandler<SSL>::JsonRpcHandler(std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<SSL>*)>> methods)
    : m_state(std::make_shared<State<SSL>>(methods))
{
}

template <bool SSL> void JsonRpcHandler<SSL>::operator()(uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req)
{
    res->onAborted([](){});

    std::string buffer;
    res->onData([res, state = m_state, buffer = std::move(buffer)](std::string_view d, bool last) mutable
    {
        buffer.append(d.data(), d.length());
        if (!last) return;

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
            state->Methods().at(method)(body.at("id"), body.at("params"), res);
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

namespace porla::Http {
    template class JsonRpcHandler<true>;
    template class JsonRpcHandler<false>;
}
