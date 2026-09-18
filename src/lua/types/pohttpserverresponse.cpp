#include "pohttpserverresponse.hpp"

#include <boost/log/trivial.hpp>

#include "../pluginstate.hpp"

using porla::Lua::Types::PoHttpServerResponse;

void PoHttpServerResponse::Register(sol::state& lua)
{
    lua.new_usertype<PoHttpServerResponse>(
        "PoHttpServerResponse",
        sol::no_constructor,
        "finish", sol::overload(
            sol::resolve<void()>(&PoHttpServerResponse::Finish),
            sol::resolve<void(const std::string&)>(&PoHttpServerResponse::Finish)
        ),
        "write", &PoHttpServerResponse::Write,
        "writeHeader", &PoHttpServerResponse::WriteHeader,
        "writeStatus", &PoHttpServerResponse::WriteStatus);
}

PoHttpServerResponse::PoHttpServerResponse(std::shared_ptr<LuaState> lua_state, uWS::HttpRequest* req, uWS::HttpResponse<false>* response, std::size_t callback_id)
    : m_callback_id(callback_id)
    , m_is_aborted(false)
    , m_response(response)
    , m_state(lua_state)
    , m_request(lua_state->lua.create_table())
{
    sol::table headers = lua_state->lua.create_table();

    for (auto [key, value] : *req)
    {
        headers[std::string{key}] = std::string{value};
    }

    m_request["method"]  = std::string{req->getMethod()};
    m_request["path"]    = std::string{req->getUrl()};
    m_request["query"]   = std::string{req->getQuery()};
    m_request["url"]     = std::string{req->getUrl()};
    m_request["headers"] = headers;
}

void PoHttpServerResponse::Setup()
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    m_response->onAborted([weak = weak_from_this()]()
    {
        auto self = weak.lock();
        if (self == nullptr) { return; }

        self->m_is_aborted = true;

        auto state = self->m_state.lock();
        if (state == nullptr) { return; }

        state->http_responses.erase(self);
    });

    m_response->onDataV2(
        [weak = weak_from_this()](std::string_view data, std::uint64_t len)
        {
            auto self = weak.lock();
            if (self == nullptr) { return; }

            self->OnData(data, len);
        });

    state->http_responses.insert(shared_from_this());
}

void PoHttpServerResponse::Finish()
{
    if (m_is_aborted) { return; }
    m_response->end();
}

void PoHttpServerResponse::Finish(const std::string& data)
{
    if (m_is_aborted) { return; }
    m_response->end(data);
}

void PoHttpServerResponse::OnData(std::string_view data, std::uint64_t len)
{
    auto state = m_state.lock();

    if (state == nullptr)
    {
        m_response->close();
        return;
    }

    if (!data.empty())
    {
        if (!m_body.has_value())
        {
            m_body.emplace();
        }

        m_body->append(data);
    }

    if (len == 0)
    {
        auto self = shared_from_this();

        self->m_request["body"] = self->m_body;

        state->http_responses.erase(self);

        boost::asio::post(state->io, [self]()
        {
            auto state = self->m_state.lock();
            if (state == nullptr) { return; }
            state->InvokeCallback(self->m_callback_id, self->m_request, self);
        });
    }
}

void PoHttpServerResponse::Write(const std::string& data)
{
    if (m_is_aborted) { return; }
    m_response->write(data);
}

void PoHttpServerResponse::WriteHeader(const std::string& key, const std::string& value)
{
    if (m_is_aborted) { return; }
    m_response->writeHeader(key, value);
}

void PoHttpServerResponse::WriteStatus(const std::string& status)
{
    if (m_is_aborted) { return; }
    m_response->writeStatus(status);
}
