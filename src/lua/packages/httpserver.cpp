#include "../packages.hpp"

#include "../pluginstate.hpp"
#include "../types/pohttpserverresponse.hpp"

using porla::Lua::LuaState;
using porla::Lua::Packages::HttpServer;
namespace
{
    using Handler = uWS::MoveOnlyFunction<void(uWS::HttpResponse<false>*, uWS::HttpRequest*)>;

    enum class Method { Get, Post, Put, Delete, Any };

    void Route(uWS::App* app, Method method, const std::string& path, Handler&& handler)
    {
        switch (method)
        {
        case Method::Get:    app->get(path, std::move(handler));  break;
        case Method::Post:   app->post(path, std::move(handler)); break;
        case Method::Put:    app->put(path, std::move(handler)); break;
        case Method::Delete: app->del(path, std::move(handler)); break;
        case Method::Any:    app->any(path, std::move(handler)); break;
        }
    }

    void RegisterRoute(sol::this_state ts, Method method, const std::string& path, sol::protected_function callback)
    {
        auto weak  = LuaState::WeakFrom(ts);
        auto state = weak.lock();

        if (state == nullptr) { return; }

        auto callback_id = state->RegisterCallback(callback, false);

        Route(state->app, method, path, [weak, callback_id](uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
        {
            auto state = weak.lock();

            if (state == nullptr)
            {
                res->writeStatus("503 Service Unavailable")->end();
                return;
            }

            auto response = std::make_shared<porla::Lua::Types::PoHttpServerResponse>(state, req, res, callback_id);
            response->Setup();
        });

        state->destructors.emplace_back([app = state->app, method, path]()
        {
            Route(app, method, path, nullptr);
        });
    }
}

sol::object HttpServer::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("get", [](sol::this_state ts, std::string path, sol::protected_function callback)
    {
        RegisterRoute(ts, Method::Get, path, callback);
    });

    tbl.set_function("post", [](sol::this_state ts, std::string path, sol::protected_function callback)
    {
        RegisterRoute(ts, Method::Post, path, callback);
    });

    tbl.set_function("put", [](sol::this_state ts, std::string path, sol::protected_function callback)
    {
        RegisterRoute(ts, Method::Put, path, callback);
    });

    tbl.set_function("delete", [](sol::this_state ts, std::string path, sol::protected_function callback)
    {
        RegisterRoute(ts, Method::Delete, path, callback);
    });

    tbl.set_function("any", [](sol::this_state ts, std::string path, sol::protected_function callback)
    {
        RegisterRoute(ts, Method::Any, path, callback);
    });

    return tbl;
}
