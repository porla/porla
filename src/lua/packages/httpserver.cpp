#include "httpserver.hpp"

#include "../pluginstate.hpp"

using porla::Lua::Packages::HttpServer;

sol::object HttpServer::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("get", [](sol::this_state ts, std::string path, sol::protected_function callback)
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return;
        }

        auto callback_id = state->next_id++;

        state->callbacks[callback_id] = callback;

        state->app->get(path, [weak, callback_id](uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
        {
            auto self = weak.lock();

            if (self == nullptr)
            {
                return;
            }

            res->onAborted([](){});

            const sol::protected_function& callback = self->callbacks.at(callback_id);

            sol::state_view lua{callback.lua_state()};

            sol::table headers = lua.create_table();

            for (auto [key, value] : *req)
            {
                headers[std::string{key}] = std::string{value};
            }

            sol::table request = lua.create_table();
            request["method"]  = std::string{req->getMethod()};
            request["path"]    = std::string{req->getUrl()};
            request["query"]   = std::string{req->getQuery()};
            request["url"]     = std::string{req->getUrl()};
            request["headers"] = headers;

            callback(request);
        });


        state->destructors.emplace_back([weak, path]()
        {
            auto state = weak.lock();
            if (!state) { return; }
            state->app->get(path, nullptr);
        });
    });

    return tbl;
}
