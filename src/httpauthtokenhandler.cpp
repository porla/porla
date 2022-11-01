#include "httpauthtokenhandler.hpp"

#include <utility>

using porla::HttpAuthTokenHandler;

HttpAuthTokenHandler::HttpAuthTokenHandler(std::string token)
    : m_token(std::move(token))
{
}

void HttpAuthTokenHandler::operator()(const std::shared_ptr<HttpContext>& ctx)
{
    auto const not_authorized = [&ctx]()
    {
        namespace http = boost::beast::http;

        http::response<http::string_body> res{http::status::unauthorized, ctx->Request().version()};
        res.set(http::field::server, "porla/1.0");
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(ctx->Request().keep_alive());
        res.body() = "Unauthorized";
        res.prepare_payload();
        return res;
    };

    const auto& auth = ctx->Request().find("Authorization");

    if (auth == ctx->Request().end()
        || auth->value().size() < 6
        || !auth->value().starts_with("Token "))
    {
        ctx->Write(not_authorized());
        return;
    }

    const auto& token = auth->value().substr(6).to_string();

    if (token != m_token)
    {
        ctx->Write(not_authorized());
        return;
    }

    ctx->Next();
}
