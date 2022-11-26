#pragma once

#include <functional>
#include <memory>

#include <boost/log/trivial.hpp>

#include "httpcontext.hpp"

namespace porla
{
    typedef std::function<void(std::shared_ptr<HttpContext> ctx)> HttpMiddleware;

    class HttpMethod
    {
    public:
        explicit HttpMethod(boost::beast::http::verb verb, std::string route, HttpMiddleware middleware)
            : m_verb(verb)
            , m_route(std::move(route))
            , m_middleware(std::move(middleware))
        {
        }

        void operator()(const std::shared_ptr<porla::HttpContext> &ctx)
        {
            if (ctx->RequestUri().path == m_route
                && ctx->Request().method() == m_verb)
            {
                m_middleware(ctx);
            }
            else
            {
                ctx->Next();
            }
        }

    private:
        boost::beast::http::verb m_verb;
        std::string m_route;
        HttpMiddleware m_middleware;
    };

    class HttpGet : public HttpMethod
    {
    public:
        explicit HttpGet(std::string route, HttpMiddleware middleware)
            : HttpMethod(
                boost::beast::http::verb::get,
                std::move(route),
                std::move(middleware))
        {
        }
    };

    class HttpPost : public HttpMethod
    {
    public:
        explicit HttpPost(std::string route, HttpMiddleware middleware)
            : HttpMethod(
            boost::beast::http::verb::post,
            std::move(route),
            std::move(middleware))
        {
        }
    };

    class HttpNotFound
    {
    public:
        void operator()(const std::shared_ptr<porla::HttpContext> &ctx)
        {
            namespace http = boost::beast::http;

            http::response<http::string_body> res{http::status::not_found, ctx->Request().version()};
            res.set(http::field::server, "porla/1.0");
            res.set(http::field::content_type, "text/plain");
            res.keep_alive(ctx->Request().keep_alive());
            res.body() = "The resource at '" + ctx->RequestUri().path + "' was not found.";
            res.prepare_payload();

            ctx->Write(res);
        }
    };
}
