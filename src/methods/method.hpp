#pragma once

#include <memory>
#include <utility>

#include <nlohmann/json.hpp>

#include "../json/all.hpp"
#include "../http/handler.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    template<typename TRes, bool SSL>
    class WriteCb
    {
    public:
        explicit WriteCb(nlohmann::json id, uWS::HttpResponse<SSL>* res)
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
        uWS::HttpResponse<SSL>* m_res;
    };

    template<typename TReq, typename TRes, bool SSL>
    class Method
    {
    public:
        void operator()(const nlohmann::json& id, const nlohmann::json& body, uWS::HttpResponse<SSL>* res)
        {
            Invoke(body.get<TReq>(), WriteCb<TRes, SSL>(id, res));
        }

    protected:
        virtual void Invoke(const TReq& req, WriteCb<TRes, SSL>) = 0;
    };
}
