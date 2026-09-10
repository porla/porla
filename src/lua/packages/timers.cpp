#include "timers.hpp"

#include "../../cron.hpp"
#include "../pluginstate.hpp"
#include "../types/pocancellable.hpp"

using porla::Lua::LuaState;
using porla::Lua::Packages::Timers;

struct PoCancellableCronSchedule : public porla::Lua::Types::PoCancellable
{
    explicit PoCancellableCronSchedule(int callback_id, int cron_schedule_id)
        : m_callback_id(callback_id)
        , m_cron_schedule_id(cron_schedule_id)
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

        if (state->callbacks.contains(m_callback_id))
        {
            state->callbacks.erase(m_callback_id);
        }

        if (state->cron_schedules.contains(m_cron_schedule_id))
        {
            state->cron_schedules.at(m_cron_schedule_id)->Cancel();
            state->cron_schedules.at(m_cron_schedule_id).reset();
            state->cron_schedules.erase(m_cron_schedule_id);
        }
    }

private:
    int m_callback_id;
    int m_cron_schedule_id;
};

struct PoCancellableTimer : public porla::Lua::Types::PoCancellable
{
    explicit PoCancellableTimer(int callback_id, int timer_id)
        : m_callback_id(callback_id)
        , m_timer_id(timer_id)
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

        if (state->callbacks.contains(m_callback_id))
        {
            state->callbacks.erase(m_callback_id);
        }

        if (state->timers.contains(m_timer_id))
        {
            state->timers.at(m_timer_id).reset();
            state->timers.erase(m_timer_id);
        }
    }

private:
    int m_callback_id;
    int m_timer_id;
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

        auto callback_id      = state->next_id++;
        auto cron_schedule_id = state->next_id++;

        state->callbacks[callback_id] = callback;
        state->cron_schedules[cron_schedule_id] = CronSchedule::Create(
            state->io,
            expression,
            [weak, callback_id]()
            {
                auto self = weak.lock();
                if (!self) return;

                auto it = self->callbacks.find(callback_id);
                if (it == self->callbacks.end()) { return; }

                it->second();
            });

        return std::make_shared<PoCancellableCronSchedule>(callback_id, cron_schedule_id);
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

        auto callback_id = state->next_id++;
        auto timer_id    = state->next_id++;

        state->callbacks[callback_id] = callback;
        state->timers[timer_id] = std::make_shared<Timer>(
            state->io,
            interval,
            [w = std::weak_ptr(state), callback_id, interval, timer_id]()
            {
                auto ops = w.lock();
                if (!ops) { return; }

                auto it = ops->callbacks.find(callback_id);
                if (it == ops->callbacks.end()) { return; }

                it->second();
            });

        return std::make_shared<PoCancellableTimer>(callback_id, timer_id);
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

        auto callback_id = state->next_id++;
        auto timer_id    = state->next_id++;

        state->callbacks[callback_id] = callback;

        state->timers[timer_id] = std::make_shared<Timer>(
            state->io,
            interval,
            [w = std::weak_ptr(state), callback_id, timer_id]()
        {
            auto ops = w.lock();
            if (!ops) { return; }

            auto it = ops->callbacks.find(callback_id);
            if (it == ops->callbacks.end()) { return; }

            sol::protected_function callback = std::move(it->second);

            ops->callbacks.erase(callback_id);

            ops->timers.at(timer_id).reset();
            ops->timers.erase(timer_id);

            callback();
        });

        return std::make_shared<PoCancellableTimer>(callback_id, timer_id);
    });

    return tbl;
}
