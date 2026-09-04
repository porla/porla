#include "../globals.hpp"

#include <chrono>
#include <boost/asio.hpp>

#include "../pluginstate.hpp"

using porla::Lua::Globals::Cron;

struct CancellableCron : public porla::Lua::Cancellable
{
    CancellableCron(std::weak_ptr<porla::Lua::LuaState> state, std::size_t id)
    {
    }

    void Cancel() override
    {
    }
};

sol::object Cron::Build(sol::state& lua)
{
    return sol::make_object(lua, [](sol::this_state L, const std::string& expression, sol::protected_function callback) -> std::optional<CancellableCron>
    {
        sol::state_view lua(L);

        auto weak_state = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak_state.lock();

        if (state == nullptr)
        {
            return std::nullopt;
        }

        auto callback_id      = state->next_id++;
        auto cron_schedule_id = state->next_id++;

        state->callbacks[callback_id] = callback;

        state->cron_schedules[cron_schedule_id] = CronSchedule::Create(
            state->io,
            expression,
            [w = weak_state, callback_id]()
            {
                auto self = w.lock();
                if (!self) return;

                auto it = self->callbacks.find(callback_id);
                if (it == self->callbacks.end()) { return; }

                sol::protected_function callback = std::move(it->second);

                self->callbacks.erase(callback_id);

                callback();
            });

        return CancellableCron(weak_state, cron_schedule_id);
    });
}
