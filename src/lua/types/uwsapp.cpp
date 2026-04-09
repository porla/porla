#include "../types.hpp"

#include <uWebSockets/App.h>

using porla::Lua::Types::UwsApp;

void UwsApp::Register(sol::state& lua)
{
    sol::table uws = lua["uws"].valid()
        ? lua["uws"].get<sol::table>()
        : lua.create_named_table("uws");

    uws["app"] = lua.new_usertype<uWS::App>(
        "uws.app",
        sol::call_constructor, sol::factories(
            []() { return std::make_shared<uWS::App>(); }
        ),
        "get",    [](std::shared_ptr<uWS::App> app, const std::string& path)
        {

        },
        "listen", [](std::shared_ptr<uWS::App> app, const std::string& host, int port, sol::protected_function callback)
        {
            auto cb = std::make_shared<sol::protected_function>(std::move(callback));

            app->listen(host, port, [cb](const auto* t)
            {
                (*cb)();
            });
        }
    );
}
