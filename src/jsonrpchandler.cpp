#include "jsonrpchandler.hpp"

#include <utility>

#include <boost/log/trivial.hpp>

using json = nlohmann::json;
using porla::JsonRpcHandler;

JsonRpcHandler::JsonRpcHandler(
    std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, std::shared_ptr<porla::HttpContext>)>> methods)
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
        BOOST_LOG_TRIVIAL(warning) << "Failed to parse JSONRPC request: " << ex.what();

        return ctx->WriteJson({
            {"error", {
                {"code", -32700},
                {"message", "Parse error"},
                {"data", ex.what()}
            }}
        });
    }

    if (!req.contains("id")
        && !req["id"].is_string()
        && !req["id"].is_number()
        && !req["id"].is_null())
    {
        return ctx->WriteJson({
            {"error", {
                {"code", -32600},
                {"message", "Invalid Request"},
                {"data", "Id is not a string, number or null"}
            }}
        });
    }

    std::string method = req.at("method").get<std::string>();

    if (m_methods.find(method) == m_methods.end())
    {
        BOOST_LOG_TRIVIAL(warning) << "Failed to find JSONRPC method '" << method << "'";

        return ctx->WriteJson({
            {"error", {
                {"code", -32601},
                {"message", "Method not found"}
            }}
        });
    }

    try
    {
        BOOST_LOG_TRIVIAL(debug) << "Executing JSONRPC method '" << method << "'";
        m_methods.at(method)(req.at("id"), req.at("params"), ctx);
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
