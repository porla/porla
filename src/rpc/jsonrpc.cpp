#include "jsonrpc.hpp"

#include <boost/log/trivial.hpp>

#include "../auth/authenticator.hpp"
#include "../json/utils.hpp"

using porla::Rpc::JsonRpc;

namespace
{
    static constexpr std::size_t kMaxBody = 8 * 1024 * 1024;
}

namespace porla
{
    struct RpcReq
    {
        std::string                   jsonrpc;
        std::string                   method;
        std::optional<nlohmann::json> params;
        std::optional<nlohmann::json> id;
    };

    NLOHMANN_JSONIFY_ALL_THINGS(
        RpcReq,
        jsonrpc,
        method,
        params,
        id)
}

class DefaultResponseWriter : public porla::Rpc::ResponseWriter
{
public:
    explicit DefaultResponseWriter(uWS::HttpResponse<false>* res, const nlohmann::json& id, std::shared_ptr<bool> aborted)
        : m_id(id)
        , m_res(res)
        , m_aborted(aborted)
        , m_responded(false)
    {
    }

    void Error(int code, const std::string& message, const nlohmann::json& data = {}) override
    {
        if (*m_aborted || m_responded) { return; }
        m_responded = true;

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

    void Header(const std::string& key, const std::string& value) override
    {
        if (*m_aborted) { return; }

        m_res->writeHeader(key, value);
    }

    void Ok(const nlohmann::json& result) override
    {
        if (*m_aborted || m_responded) { return; }
        m_responded = true;

        m_res->end(json({
            {"jsonrpc", "2.0"},
            {"id", m_id},
            {"result", result}
        }).dump());
    }

private:
    nlohmann::json            m_id;
    uWS::HttpResponse<false>* m_res;
    std::shared_ptr<bool>     m_aborted;
    bool                      m_responded;
};

JsonRpc::JsonRpc(std::shared_ptr<Auth::Authenticator> authenticator)
    : m_authenticator(std::move(authenticator))
{
}

std::shared_ptr<JsonRpc> JsonRpc::Create(std::shared_ptr<Auth::Authenticator> authenticator)
{
    return std::shared_ptr<JsonRpc>(new JsonRpc(std::move(authenticator)));
}

std::function<void(uWS::HttpResponse<false>*, uWS::HttpRequest*)> JsonRpc::HttpHandler()
{
    return [weak = weak_from_this()](uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
    {
        auto jsonrpc = weak.lock();

        if (jsonrpc == nullptr)
        {
            res->writeStatus("503 Unavailable")
                ->end("JSONRPC server gone to sleep");

            return;
        }

        const auto auth_context = jsonrpc->m_authenticator->Authenticate(req);

        auto aborted = std::make_shared<bool>(false);
        auto buffer = std::make_shared<std::string>();

        res->onAborted([aborted] { *aborted = true; });

        res->onData([aborted, buffer, res, weak, auth_context](std::string_view data, bool last)
        {
            if (buffer->size() + data.size() > kMaxBody)
            {
                res->writeStatus("413 Payload  Too Large")->end({}, true);
                *aborted = true;
                return;
            }

            buffer->append(data);
            if (!last) return;

            auto jsonrpc = weak.lock();

            if (jsonrpc == nullptr)
            {
                res->writeStatus("503 Unavailable")
                    ->end("JSONRPC server gone to sleep");

                return;
            }

            // From here on out we always respond with valid JSON.

            res->writeStatus("200 OK")
                ->writeHeader("Content-Type", "application/json");

            RpcReq req;

            try
            {
                req = json::parse(*buffer).get<RpcReq>();
            }
            catch (const std::exception& ex)
            {
                BOOST_LOG_TRIVIAL(warning) << "Failed to parse JSONRPC request: " << ex.what();

                json error = {
                    {"error", {
                        {"code", -32700},
                        {"message", "Parse error"},
                        {"data", ex.what()}
                    }}
                };

                res->end(error.dump());

                return;
            }

            if (req.id.has_value()
                && !req.id->is_string()
                && !req.id->is_number())
            {
                res->end(json({
                    {"error", {
                        {"code", -32600},
                        {"message", "Invalid Request"},
                        {"data", "Id is not a string, number or null"}
                    }}
                }).dump());

                return;
            }

            auto method = jsonrpc->m_methods.find(req.method);

            if (method == jsonrpc->m_methods.end())
            {
                BOOST_LOG_TRIVIAL(debug) << "Failed to find JSONRPC method '" << req.method << "'";

                res->end(json({
                    {"error", {
                        {"code", -32601},
                        {"message", "Method not found"}
                    }}
                }).dump());

                return;
            }

            try
            {
                BOOST_LOG_TRIVIAL(debug) << "Executing JSONRPC method '" << req.method << "'";

                auto writer = std::make_shared<DefaultResponseWriter>(res, req.id.value_or(json()), aborted);

                if (!method->second->CanInvoke(auth_context))
                {
                    writer->Error(1001, "Invocation not allowed");
                    return;
                }

                method->second->Invoke(req.params.value_or(json()), writer);
            }
            catch (const std::exception& ex)
            {
                BOOST_LOG_TRIVIAL(error) << "Error when executing JSONRPC method '" << req.method << "': " << ex.what();

                res->end(json({
                    {"error", {
                        {"code", -32603},
                        {"message", "Internal error"},
                        {"data", ex.what()}
                    }}
                }).dump());
            }
        });
    };
}
