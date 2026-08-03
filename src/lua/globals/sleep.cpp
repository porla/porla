#include "../globals.hpp"

#include <chrono>
#include <boost/asio.hpp>

#include "../registry.hpp"

using porla::Lua::Globals::Sleep;

sol::object Sleep::Build(sol::state& lua)
{
    sol::table sleep = lua.create_table();

    return sol::make_object(lua, sol::yielding([](sol::this_state L, double seconds)
    {
        sol::state_view lua(L);

        auto io = lua.registry()["io"].get<porla::Lua::Registry::BoostIoContext>().io;

        auto timer = std::make_shared<boost::asio::steady_timer>(*io);
        timer->expires_after(std::chrono::milliseconds(static_cast<long long>(seconds * 1000)));

        timer->async_wait([L, timer](boost::system::error_code ec)
        {
            int results = 0;
            lua_resume(L, nullptr, 0, &results);
        });
    }));
}
