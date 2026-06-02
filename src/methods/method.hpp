#pragma once

#include <memory>
#include <utility>

#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>

#include "../json/all.hpp"
#include "../http/handler.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    template<typename TRes>
    class WriteCb
    {
    public:
        explicit WriteCb(nlohmann::json id, uWS::HttpResponse<false>* res)
            : m_id(std::move(id))
            , m_res(res)
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

        void Error(int code, const std::string_view& message, const json& data = {})
        {
            m_res->end(json({
                {"jsonrpc", "2.0"},
                {"id", m_id},
                {"error", {
                    {"code", code},
                    {"message", message},
                    {"data", data}
                }}
            }).dump());
        }

        void Header(const std::string& key, const std::string& val)
        {
            m_res->writeHeader(key, val);
        }

        void Ok(const json& result)
        {
            m_res->end(json({
                {"jsonrpc", "2.0"},
                {"id", m_id},
                {"result", result}
            }).dump());
        }

    private:
        nlohmann::json m_id;
        uWS::HttpResponse<false>* m_res;
    };

    template<typename TReq, typename TRes>
    class Method
    {
    public:
        typedef std::optional<jwt::decoded_jwt<jwt::traits::nlohmann_json>> Token;

        void operator()(const nlohmann::json& id, const nlohmann::json& body, uWS::HttpResponse<false>* res, Token token)
        {
            if (!CanInvoke(token))
            {
                res->end(json({
                    {"jsonrpc", "2.0"},
                    {"id", id},
                    {"error", {
                        {"code", 1001}
                    }}
                }).dump());

                return;
            }

            Invoke(body.get<TReq>(), WriteCb<TRes>(id, res));
        }

    protected:
        virtual bool CanInvoke(Token token)
        {
            return false;
        }

        virtual void Invoke(const TReq& req, WriteCb<TRes>) = 0;
    };
}
