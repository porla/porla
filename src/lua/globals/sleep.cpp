#include "../globals.hpp"

#include <chrono>
#include <boost/asio.hpp>

#include "../pluginstate.hpp"

using porla::Lua::Globals::Sleep;

sol::object Sleep::Build(sol::state& lua)
{
    return sol::make_object(lua, [](sol::this_state L, double seconds, sol::protected_function callback)
    {
        sol::state_view lua(L);

        auto weak_state = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak_state.lock();

        if (state == nullptr)
        {
            return;
        }

        auto callback_id = state->next_id++;
        auto timer_id    = state->next_id++;

        state->callbacks[callback_id] = callback;

        state->steady_timers[timer_id] = std::make_shared<boost::asio::steady_timer>(state->io);
        state->steady_timers[timer_id]->expires_after(std::chrono::milliseconds(static_cast<long long>(seconds * 1000)));
        state->steady_timers[timer_id]->async_wait([w = std::weak_ptr(state), callback_id, timer_id](boost::system::error_code ec)
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
