#pragma once

#include <memory>

#include <nlohmann/json.hpp>
#include <utility>

#include "../json/all.hpp"
#include "../httpcontext.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    template<typename TRes>
    class WriteCb
    {
    public:
        explicit WriteCb(std::shared_ptr<porla::HttpContext> ctx)
            : m_ctx(std::move(ctx))
        {
        }

        void operator()(const TRes& res)
        {
            json d = res;
            m_ctx->WriteJson(d);
        }
    
    private:
        std::shared_ptr<porla::HttpContext> m_ctx;
    };

    template<typename TReq, typename TRes>
    class MethodT
    {
    public:
        void operator()(const std::shared_ptr<porla::HttpContext> &ctx)
        {
            if (ctx->Request().target() == m_route
                && ctx->Request().method() == m_verb)
            {
                json req;
                req = json::parse(ctx->Request().body());

                Invoke(req.get<TReq>(), WriteCb<TRes>(ctx));
            }
            else
            {
                ctx->Next();
            }
        }

    protected:
        explicit MethodT(boost::beast::http::verb verb, std::string route)
            : m_verb(verb)
            , m_route(std::move(route))
        {
        }


        virtual void Invoke(TReq const& req, WriteCb<TRes>) {};

    private:
        boost::beast::http::verb m_verb;
        std::string m_route;
    };

    template<typename TRes>
    class Method
    {
    public:
        void operator()(const std::shared_ptr<porla::HttpContext> &ctx)
        {
            if (ctx->Request().target() == m_route
                && ctx->Request().method() == m_verb)
            {
                Invoke(WriteCb<TRes>(ctx));
            }
            else
            {
                ctx->Next();
            }
        }

    protected:
        explicit Method(boost::beast::http::verb verb, std::string route)
            : m_verb(verb)
            , m_route(std::move(route))
        {
        }


        virtual void Invoke(WriteCb<TRes>) {};

    private:
        boost::beast::http::verb m_verb;
        std::string m_route;
    };
}
