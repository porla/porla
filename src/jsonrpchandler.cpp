#include "jsonrpchandler.hpp"

#include <utility>

#include <boost/log/trivial.hpp>

using json = nlohmann::json;
using porla::JsonRpcHandler;

JsonRpcHandler::JsonRpcHandler(
    std::map<std::string, std::function<void(const nlohmann::json&, std::shared_ptr<porla::HttpContext>)>> methods)
    : m_methods(std::move(methods))
{
}

void JsonRpcHandler::operator()(const std::shared_ptr<porla::HttpContext> &ctx)
{
    json req = {};

    try
    {
        req = json::parse(ctx->Request().body());
    }
    catch (const std::exception& ex)
    {
        return ctx->WriteJson({
            {"error", {
                {"code", -32700},
                {"message", "Parse error"},
                {"data", ex.what()}
            }}
        });
    }

    std::string method = req.at("method").get<std::string>();

    if (m_methods.find(method) == m_methods.end())
    {
        return ctx->WriteJson({
            {"error", {
                {"code", -32601},
                {"message", "Method not found"}
            }}
        });
    }

    try
    {
        m_methods.at(method)(req.at("params"), ctx);
    }
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(error) << "Error when executing JSONRPC method '" << method << "': " << ex.what();

        ctx->WriteJson({
            {"error", {
                {"code", -32603},
                {"message", "Internal error"},
                {"data", ex.what()}
            }}
        });
    }
}
