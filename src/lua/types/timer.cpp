#include "../types.hpp"

#include <boost/asio.hpp>

#include "../registry.hpp"

using porla::Lua::Types::Timer;

void Timer::Register(sol::state& lua)
{
    sol::table timer = lua["timer"].valid()
        ? lua["timer"].get<sol::table>()
        : lua.create_named_table("timer");

    timer["sleep"] = sol::yielding([](sol::this_state L, int duration_ms)
    {
        sol::state_view lua(L);

        auto io = lua.registry()["io"].get<porla::Lua::Registry::BoostIoContext>().io;
        auto timer = std::make_shared<boost::asio::steady_timer>(*io, std::chrono::milliseconds(duration_ms));

        timer->async_wait([L, timer](boost::system::error_code ec)
        {
            int results = 0;
            lua_resume(L, nullptr, 0, &results);
        });
    });
}
