#include "jsonrpc.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/log/trivial.hpp>
#include <zlib.h>

#include "../auth/authenticator.hpp"
#include "../json/utils.hpp"

using porla::Rpc::JsonRpc;

namespace
{
    static constexpr std::size_t kMaxBody = 8 * 1024 * 1024;
    static constexpr std::size_t kMinGzipSize = 1024; // don't compress small responses

    std::optional<std::string> Gzip(std::string_view input)
    {
        z_stream zs{};

        if (deflateInit2(&zs, 6, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        {
            return std::nullopt;
        }

        const auto size = deflateBound(&zs, input.size());

        std::string output(size, '\0');

        zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
        zs.avail_in = static_cast<uInt>(input.size());
        zs.next_out = reinterpret_cast<Bytef*>(output.data());
        zs.avail_out = static_cast<uInt>(output.size());

        const int res = deflate(&zs, Z_FINISH);
        deflateEnd(&zs);

        if (res != Z_STREAM_END)
        {
            return std::nullopt;
        }

        output.resize(zs.total_out);

        return output;
    }

    // Every reply goes through here. Invalid UTF-8 (from a tracker message, or from the request
    // itself, quoted in a parse error) becomes U+FFFD instead of making dump() throw.
    std::string SerializeJson(const nlohmann::json& reply)
    {
        return reply.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    }
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
}

class DefaultResponseWriter : public porla::Rpc::ResponseWriter
{
public:
    explicit DefaultResponseWriter(uWS::HttpResponse<false>* res, const nlohmann::json& id, std::shared_ptr<bool> aborted, bool gzip)
        : m_id(id)
        , m_res(res)
        , m_aborted(aborted)
        , m_responded(false)
        , m_gzip(gzip)
    {
    }

    void Error(int code, const std::string& message, const nlohmann::json& data = {}) override
    {
        if (*m_aborted || m_responded) { return; }

        const std::string body = SerializeJson(json({
            {"jsonrpc", "2.0"},
            {"id", m_id},
            {"error", {
                {"code", code},
                {"message", message},
                {"data", data}
            }}
        }));

        m_responded = true;

        m_res->end(body);
    }

    void Header(const std::string& key, const std::string& value) override
    {
        if (*m_aborted) { return; }

        m_res->writeHeader(key, value);
    }

    void Ok(const nlohmann::json& result) override
    {
        if (*m_aborted || m_responded) { return; }

        const std::string body = SerializeJson(json({
            {"jsonrpc", "2.0"},
            {"id", m_id},
            {"result", result}
        }));

        m_responded = true;

        if (m_gzip && body.size() >= kMinGzipSize)
        {
            if (const auto gzipped = Gzip(body))
            {
                m_res->writeHeader("Content-Encoding", "gzip");
                m_res->writeHeader("Vary", "Accept-Encoding");
                m_res->end(gzipped.value());
                return;
            }
        }

        m_res->end(body);
    }

private:
    nlohmann::json            m_id;
    uWS::HttpResponse<false>* m_res;
    std::shared_ptr<bool>     m_aborted;
    bool                      m_responded;
    bool                      m_gzip;
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

        // Browsers say where a request comes from. porla answers no CORS preflight, so a browser
        // request from another site - or another port or subdomain of this one ("same-site") - can
        // only be a "simple" request a page sends to act with the user's cookie (or to run
        // auth.init on a fresh install). The web UI is same-origin; scripts send no such header.
        if (const auto fetch_site = req->getHeader("sec-fetch-site");
            boost::algorithm::iequals(fetch_site, "cross-site") || boost::algorithm::iequals(fetch_site, "same-site"))
        {
            res->writeStatus("403 Forbidden")->end("Forbidden", true);
            return;
        }

        auto auth_context = jsonrpc->m_authenticator->Authenticate(req);

        auto user_agent = req->getHeader("user-agent").substr(0, 512);

        if (user_agent.size() == 512)
        {
            while (!user_agent.empty() && (static_cast<unsigned char>(user_agent.back()) & 0xC0) == 0x80)
            {
                user_agent.remove_suffix(1); // continuation bytes
            }

            if (!user_agent.empty() && static_cast<unsigned char>(user_agent.back()) >= 0xC0)
            {
                user_agent.remove_suffix(1); // dangling lead byte
            }
        }

        auth_context.remote_address = std::string(res->getRemoteAddressAsText());
        auth_context.user_agent = std::string(user_agent);

        const bool accepts_gzip = boost::algorithm::icontains(req->getHeader("accept-encoding"), "gzip");

        auto aborted = std::make_shared<bool>(false);
        auto buffer = std::make_shared<std::string>();

        res->onAborted([aborted] { *aborted = true; });

        res->onData([aborted, buffer, res, weak, auth_context, accepts_gzip](std::string_view data, bool last)
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

            // every reply, errors included, goes through a writer so it carries jsonrpc and id
            const auto fail = [&](const json& id, int code, const std::string& message, const json& data = {})
            {
                std::make_shared<DefaultResponseWriter>(res, id, aborted, false)->Error(code, message, data);
            };

            json body;

            try
            {
                body = Json::Parse(*buffer);
            }
            catch (const std::exception& ex)
            {
                BOOST_LOG_TRIVIAL(warning) << "Failed to parse JSONRPC request: " << ex.what();
                fail(nullptr, -32700, "Parse error", ex.what());
                return;
            }

            const auto id_it = body.is_object() ? body.find("id") : body.end();
            const bool id_ok = !body.is_object() || id_it == body.end() || id_it->is_null() || id_it->is_string() || id_it->is_number();
            const json id    = body.is_object() && id_it != body.end() && id_ok ? *id_it : json();

            if (!body.is_object()
                || !id_ok
                || !body.contains("jsonrpc") || !body["jsonrpc"].is_string()
                || !body.contains("method")  || !body["method"].is_string())
            {
                fail(id, -32600, "Invalid Request", !id_ok
                    ? "Id is not a string, number or null"
                    : "A request is an object with string jsonrpc and method");
                return;
            }

            RpcReq req{
                .jsonrpc = body["jsonrpc"].get<std::string>(),
                .method  = body["method"].get<std::string>(),
                .params  = body.contains("params") ? std::optional(body["params"]) : std::nullopt,
                .id      = id_it != body.end() ? std::optional(*id_it) : std::nullopt
            };

            auto method = jsonrpc->m_methods.find(req.method);

            if (method == jsonrpc->m_methods.end())
            {
                BOOST_LOG_TRIVIAL(debug) << "Failed to find JSONRPC method '" << req.method << "'";
                fail(id, -32601, "Method not found");
                return;
            }

            // auth responses carry tokens so compressing secrets next to request input
            // over TLS invites BREACH-style attacks
            const bool gzip = accepts_gzip && !req.method.starts_with("auth.");

            auto writer = std::make_shared<DefaultResponseWriter>(res, req.id.value_or(json()), aborted, gzip);

            try
            {
                BOOST_LOG_TRIVIAL(debug) << "Executing JSONRPC method '" << req.method << "'";

                if (!method->second.Allows(auth_context) || !method->second.method->CanInvoke(auth_context))
                {
                    writer->Error(1001, "Invocation not allowed");
                    return;
                }

                method->second.method->Invoke(auth_context, req.params.value_or(json()), writer);
            }
            catch (const std::exception& ex)
            {
                BOOST_LOG_TRIVIAL(error) << "Error when executing JSONRPC method '" << req.method << "': " << ex.what();
                writer->Error(-32603, "Internal error", ex.what());
            }
        });
    };
}
