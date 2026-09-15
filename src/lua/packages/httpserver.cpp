#include "httpserver.hpp"

#include "../pluginstate.hpp"
#include "../types/pohttpserverresponse.hpp"

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

        if (state == nullptr) { return; }

        auto callback_id = state->RegisterCallback(callback, false);

        state->app->get(path, [weak, callback_id](uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
        {
            auto state = weak.lock();

            if (state == nullptr) { return; }

            auto response = std::make_shared<Types::PoHttpServerResponse>(state, req, res, callback_id);
            response->Setup();
        });

        state->destructors.emplace_back([weak, path]()
        {
            auto state = weak.lock();
            if (!state) { return; }
            state->app->get(path, nullptr);
        });
    });

    tbl.set_function("post", [](sol::this_state ts, std::string path, sol::protected_function callback)
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr) { return; }

        auto callback_id = state->RegisterCallback(callback, false);

        state->app->post(path, [weak, callback_id](uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
        {
            auto state = weak.lock();

            if (state == nullptr) { return; }

            auto response = std::make_shared<Types::PoHttpServerResponse>(state, req, res, callback_id);
            response->Setup();
        });

        state->destructors.emplace_back([weak, path]()
        {
            auto state = weak.lock();
            if (!state) { return; }
            state->app->post(path, nullptr);
        });
    });

    return tbl;
}
