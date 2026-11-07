#include "../packages.hpp"

#include <cmath>
#include <limits>

#include "../pluginstate.hpp"
#include "../types/pocancellable.hpp"
#include "../types/poerror.hpp"

using porla::Lua::Error;
using porla::Lua::LuaState;
using porla::Lua::Packages::Timers;

namespace
{
    // taken as a double because Lua computes durations like 1.5 * 60 * 1000 as floats, and an int parameter
    // would both reject those and silently wrap values past 2^31
    int Milliseconds(double ms)
    {
        if (!std::isfinite(ms) || std::trunc(ms) != ms || ms < 10 || ms > std::numeric_limits<int>::max())
        {
            throw Error(porla::Errors::Code::invalid_params, "ms must be a whole number from 10 to 2147483647");
        }

        return static_cast<int>(ms);
    }
}

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

        const auto timer_id = state->RegisterCron(expression, callback);

        return std::make_shared<Types::PoCancellable>(
            [timer_id](LuaState& s) { s.CancelTimer(timer_id); });
    });

    tbl.set_function("epoch", []()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    });

    tbl.set_function("interval", [](sol::this_state ts, double ms, sol::protected_function callback) -> std::shared_ptr<Types::PoCancellable>
    {
        auto state = LuaState::From(ts);

        if (state == nullptr)
        {
            return nullptr;
        }

        const auto timer_id = state->RegisterTimer(Milliseconds(ms), callback, false);

        return std::make_shared<Types::PoCancellable>(
            [timer_id](LuaState& s) { s.CancelTimer(timer_id); });
    });

    tbl.set_function("timeout", [](sol::this_state ts, double ms, sol::protected_function callback) -> std::shared_ptr<Types::PoCancellable>
    {
        auto state = LuaState::From(ts);

        if (state == nullptr)
        {
            return nullptr;
        }

        const auto timer_id = state->RegisterTimer(Milliseconds(ms), callback, true);

        return std::make_shared<Types::PoCancellable>(
            [timer_id](LuaState& s) { s.CancelTimer(timer_id); });
    });

    return tbl;
}
