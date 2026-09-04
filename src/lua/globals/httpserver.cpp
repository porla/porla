#include "../globals.hpp"

#include <boost/log/trivial.hpp>

#include "../registry.hpp"
#include "../pluginstate.hpp"

using porla::Lua::Globals::HttpServer;

class HttpResponseHandle : std::enable_shared_from_this<HttpResponseHandle>
{
public:
    explicit HttpResponseHandle(uWS::HttpResponse<false>* res)
        : m_res(res)
    {
    }

    ~HttpResponseHandle()
    {
        if (m_res)
        {
            m_res->end();
        }
    }

    void Setup()
    {
        m_res->onAborted([w = weak_from_this()]()
        {
            auto self = w.lock();
            if (!self) { return; }
            self->m_aborted = true;
        });
    }

    void end()
    {
        if (m_aborted) { return; }
        m_res->end();
    }

    void end(std::string_view data)
    {
        if (m_aborted) { return; }
        m_res->end(data);
    }

    void write(std::string_view data)
    {
        if (m_aborted) { return; }
        m_res->write(data);
    }

    void writeHeader(std::string_view key, std::string_view val)
    {
        if (m_aborted) { return; }
        m_res->writeHeader(key, val);
    }

    void writeStatus(std::string_view status)
    {
        if (m_aborted) { return; }
        m_res->writeStatus(status);
    }

private:
    bool m_aborted = false;
    uWS::HttpResponse<false>* m_res;
};

sol::object HttpServer::Build(sol::state& lua)
{
    lua.new_usertype<uWS::HttpRequest>(
        "http_server.Request",
        sol::no_constructor);

    lua.new_usertype<HttpResponseHandle>(
        "http_server.Response",
        sol::no_constructor,
        "finish", sol::overload(
            [](HttpResponseHandle& h) { h.end(); },
            [](HttpResponseHandle& h, std::string_view data) { h.end(data); }
        ),
        "write", &HttpResponseHandle::write,
        "writeHeader", &HttpResponseHandle::writeHeader,
        "writeStatus", &HttpResponseHandle::writeStatus);

    sol::table http_server = lua.create_table();

    http_server["get"] = [](sol::this_state L, const std::string& pattern, sol::protected_function callback)
    {
        sol::state_view lua(L);

        BOOST_LOG_TRIVIAL(trace) << "Attaching HTTP GET handler for " << pattern;

        auto weak_state = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak_state.lock();

        if (state == nullptr)
        {
            return;
        }

        auto callback_id = state->next_id++;

        state->callbacks[callback_id] = callback;

        state->app->get(pattern, [w = weak_state, callback_id](uWS::HttpResponse<false>* res, auto req)
        {
            auto state = w.lock();
            if (!state) { return; }

            auto it = state->callbacks.find(callback_id);
            if (it == state->callbacks.end()) { return; }

            sol::protected_function callback = std::move(it->second);

            state->callbacks.erase(callback_id);

            auto response = std::make_shared<HttpResponseHandle>(res);
            response->Setup();

            callback(req, response);
        });

        state->destructors.emplace_back([w = weak_state, p = pattern]()
        {
            auto state = w.lock();
            if (!state) { return; }

            state->app->get(p, nullptr);
        });
    };

    return http_server;
}
