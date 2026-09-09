#include "timers.hpp"

#include "../pluginstate.hpp"

using porla::Lua::Packages::Timers;

sol::object Timers::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("timeout", [](sol::this_state ts, double interval, sol::protected_function callback)
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return sol::nil;
        }

        auto callback_id = state->next_id++;
        auto timer_id    = state->next_id++;

        state->callbacks[callback_id] = callback;

        state->steady_timers[timer_id] = std::make_shared<boost::asio::steady_timer>(state->io);
        state->steady_timers[timer_id]->expires_after(std::chrono::milliseconds(static_cast<long long>(interval * 1000)));
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

        return sol::nil;
    });

    return tbl;
}
