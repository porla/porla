#include "timers.hpp"

#include "../../cron.hpp"
#include "../pluginstate.hpp"
#include "../types/pocancellable.hpp"

using porla::Lua::LuaState;
using porla::Lua::Packages::Timers;

struct PoCancellableCronSchedule : public porla::Lua::Types::PoCancellable
{
    explicit PoCancellableCronSchedule(std::size_t cron_schedule_id)
        : m_cron_schedule_id(cron_schedule_id)
    {
    }

    void Cancel(sol::this_state ts) override
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return;
        }

        state->CancelCronSchedule(m_cron_schedule_id);
    }

private:
    std::size_t m_cron_schedule_id;
};

struct PoCancellableTimer : public porla::Lua::Types::PoCancellable
{
    explicit PoCancellableTimer(std::size_t timer_id)
        : m_timer_id(timer_id)
    {
    }

    void Cancel(sol::this_state ts) override
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return;
        }

        state->CancelTimer(m_timer_id);
    }

private:
    std::size_t m_timer_id;
};

sol::object Timers::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("cron", [](sol::this_state ts, std::string expression, sol::protected_function callback) -> std::shared_ptr<Types::PoCancellable>
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return nullptr;
        }

        auto cron_schedule_id = state->RegisterCronSchedule(expression, callback);

        return std::make_shared<PoCancellableCronSchedule>(cron_schedule_id);
    });

    tbl.set_function("epoch", []()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    });

    tbl.set_function("interval", [](sol::this_state ts, int interval, sol::protected_function callback) -> std::shared_ptr<Types::PoCancellable>
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return nullptr;
        }

        const auto timer_id = state->RegisterTimer(interval, callback, false);

        return std::make_shared<PoCancellableTimer>(timer_id);
    });

    tbl.set_function("timeout", [](sol::this_state ts, int interval, sol::protected_function callback) -> std::shared_ptr<Types::PoCancellable>
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return nullptr;
        }

        const auto timer_id = state->RegisterTimer(interval, callback, true);

        return std::make_shared<PoCancellableTimer>(timer_id);
    });

    return tbl;
}
