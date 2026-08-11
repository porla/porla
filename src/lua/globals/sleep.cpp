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

        // Anchor the coroutine in the registry for as long as the timer is
        // pending. Nothing else keeps it alive once the plugin stops tracking
        // it - the unload path abandons still-suspended coroutines when it
        // times out - and resuming a collected thread is a use after free.
        lua_pushthread(L);
        const int thread_ref = luaL_ref(L, LUA_REGISTRYINDEX);

        auto timer = std::make_shared<boost::asio::steady_timer>(*io);
        timer->expires_after(std::chrono::milliseconds(static_cast<long long>(seconds * 1000)));

        timer->async_wait([L, timer, thread_ref](boost::system::error_code ec)
        {
            int results = 0;
            lua_resume(L, nullptr, 0, &results);

            // After the resume: if the handler slept again it took a ref of its
            // own, so dropping ours here is always correct.
            luaL_unref(L, LUA_REGISTRYINDEX, thread_ref);
        });
    }));
}
