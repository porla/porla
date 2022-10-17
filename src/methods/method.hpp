#pragma once

#include <memory>
#include <utility>

#include <nlohmann/json.hpp>

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

        void operator()(const json& j)
        {
            m_ctx->WriteJson(j);
        }
    
    private:
        std::shared_ptr<porla::HttpContext> m_ctx;
    };

    template<typename TReq, typename TRes>
    class Method
    {
    public:
        void operator()(const nlohmann::json& body, std::shared_ptr<porla::HttpContext> ctx)
        {
            Invoke(body.get<TReq>(), WriteCb<TRes>(std::move(ctx)));
        }

    protected:
        virtual void Invoke(const TReq& req, WriteCb<TRes>) = 0;
    };
}
