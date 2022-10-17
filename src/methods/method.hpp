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
            Ok(d);
        }

        void operator()(const json& j)
        {
            Ok(j);
        }

        void Error(int code, const std::string_view& message)
        {
            m_ctx->WriteJson({
                {"jsonrpc", "2.0"},
                {"error", {
                    {"code", code},
                    {"message", message}
                }}
            });
        }

        void Ok(const json& result)
        {
            m_ctx->WriteJson({
                {"jsonrpc", "2.0"},
                {"result", result}
            });
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
