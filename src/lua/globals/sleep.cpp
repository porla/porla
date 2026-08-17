#include "../globals.hpp"

#include <chrono>
#include <boost/asio.hpp>

#include "../registry.hpp"

using porla::Lua::Globals::Sleep;

sol::object Sleep::Build(sol::state& lua)
{
    return sol::make_object(lua, [](sol::this_state L, double seconds, sol::protected_function callback)
    {
        sol::state_view lua(L);

        auto io  = lua.registry()["io"].get<porla::Lua::Registry::BoostIoContext>().io;
        auto ops = lua.registry()["ops"].get<std::shared_ptr<porla::Lua::Registry::Ops>>();

        auto callback_id = ops->next_id++;
        auto timer_id    = ops->next_id++;

        ops->callbacks[callback_id] = callback;

        ops->steady_timers[timer_id] = std::make_shared<boost::asio::steady_timer>(*io);
        ops->steady_timers[timer_id]->expires_after(std::chrono::milliseconds(static_cast<long long>(seconds * 1000)));
        ops->steady_timers[timer_id]->async_wait([w = std::weak_ptr(ops), callback_id, timer_id](boost::system::error_code ec)
        {
            if (ec) { return; }

            auto ops = w.lock();
            if (!ops) { return; }

            auto it = ops->callbacks.find(callback_id);
            if (it == ops->callbacks.end()) { return; }

            sol::protected_function callback = std::move(it->second);

            ops->callbacks.erase(callback_id);
            ops->steady_timers.erase(timer_id);

            callback();
        });
    });
}
