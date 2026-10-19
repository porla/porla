#include "../packages.hpp"

#include "../../cron.hpp"
#include "../pluginstate.hpp"
#include "../types/pocancellable.hpp"

using porla::Lua::LuaState;
using porla::Lua::Packages::Timers;

sol::object Timers::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("cron", [](sol::this_state ts, std::string expression, sol::protected_function callback) -> std::shared_ptr<Types::PoCancellable>
    {
        auto state = LuaState::From(ts);

        if (state == nullptr)
        {
            return nullptr;
        }

        auto cron_schedule_id = state->RegisterCronSchedule(expression, callback);

        return std::make_shared<Types::PoCancellable>(
            [cron_schedule_id](LuaState& s) { s.CancelCronSchedule(cron_schedule_id); });
    });

    tbl.set_function("epoch", []()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    });

    tbl.set_function("interval", [](sol::this_state ts, int interval, sol::protected_function callback) -> std::shared_ptr<Types::PoCancellable>
    {
        auto state = LuaState::From(ts);

        if (state == nullptr)
        {
            return nullptr;
        }

        const auto timer_id = state->RegisterTimer(interval, callback, false);

        return std::make_shared<Types::PoCancellable>(
            [timer_id](LuaState& s) { s.CancelTimer(timer_id); });
    });

    tbl.set_function("timeout", [](sol::this_state ts, int interval, sol::protected_function callback) -> std::shared_ptr<Types::PoCancellable>
    {
        auto state = LuaState::From(ts);

        if (state == nullptr)
        {
            return nullptr;
        }

        const auto timer_id = state->RegisterTimer(interval, callback, true);

        return std::make_shared<Types::PoCancellable>(
            [timer_id](LuaState& s) { s.CancelTimer(timer_id); });
    });

    return tbl;
}
