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
    json req = json::parse(ctx->Request().body());
    std::string method = req.at("method").get<std::string>();

    if (m_methods.find(method) == m_methods.end())
    {
        ctx->WriteJson({
            {}
        });

        return;
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
                {"code", -2332},
                {"message", ex.what()}
            }}
        });
    }
}
