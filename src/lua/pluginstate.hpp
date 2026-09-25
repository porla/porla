#pragma once

#include <map>
#include <memory>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/signals2.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>
#include <uWebSockets/App.h>

#include "../config.hpp"
#include "../cron.hpp"
#include "../curlmulti.hpp"
#include "../rpc/jsonrpc.hpp"
#include "../sessions.hpp"
#include "../timer.hpp"

#include "types/pohttpserverresponse.hpp"

namespace porla::Lua
{
    struct LuaState : public std::enable_shared_from_this<LuaState>
    {
        explicit LuaState(boost::asio::io_context& io, boost::asio::thread_pool& hash_pool, porla::Config& cfg, porla::Sessions& sessions, sol::state_view lua)
            : io(io)
            , cfg(cfg)
            , sessions(sessions)
            , lua(lua)
            , sodium_hash_pool(hash_pool)
        {
        }

        ~LuaState()
        {
            for (auto& [ _, cron_schedule ] : m_cron_schedules)
            {
                cron_schedule.handle->Cancel();
            }

            for (auto& [ _, signal ] : m_signals)
            {
                signal.disconnect();
            }

            for (auto& [ _, timer ] : m_timers)
            {
                timer.handle->Cancel();
            }

            for (auto& dtor : destructors)
            {
                dtor();
            }
        }

        void CancelCronSchedule(std::size_t cron_schedule_id)
        {
            CancelEntry(m_cron_schedules, cron_schedule_id);
        }

        void CancelScopedConnection(std::size_t connection_id)
        {
            const auto entry = m_signals.find(connection_id);
            if (entry == m_signals.end()) { return; }

            const auto connection = std::move(entry->second);

            m_signals.erase(entry);

            connection.disconnect();
        }


        void CancelTimer(std::size_t timer_id)
        {
            CancelEntry(m_timers, timer_id);
        }

        template <typename... Args>
        void InvokeCallback(std::size_t callback_id, Args&&... args)
        {
            const auto entry = m_callbacks.find(callback_id);

            if (entry == m_callbacks.end())
            {
                BOOST_LOG_TRIVIAL(error) << "plugin[" << plugin_id << "] No callback with id " << callback_id;
                return;
            }

            CallbackRef ref = entry->second;

            if (ref.one_shot)
            {
                m_callbacks.erase(entry);
            }

            sol::protected_function_result result = ref.callback(std::forward<Args>(args)...);

            if (!result.valid())
            {
                sol::error err = result;
                BOOST_LOG_TRIVIAL(error) << "plugin[" << plugin_id << "] Error when invoking callback: " << err.what();
            }
        }

        boost::asio::any_io_executor IoExecutor() const { return io.get_executor(); }

        std::size_t RegisterCallback(sol::main_protected_function func, bool one_shot)
        {
            const auto id = NextId();
            m_callbacks[id] = CallbackRef{.callback = std::move(func), .one_shot = one_shot};
            return id;
        }

        std::size_t RegisterCronSchedule(const std::string& expression, sol::main_protected_function func)
        {
            const auto cron_schedule_id = NextId();
            const auto callback_id      = RegisterCallback(std::move(func), false);

            auto cron_schedule = CronSchedule::Create(
                io,
                expression,
                [w = weak_from_this(), cron_schedule_id]()
                {
                    if (auto self = w.lock())
                    {
                        self->TriggerCronSchedule(cron_schedule_id);
                    }
                });

            m_cron_schedules.emplace(cron_schedule_id, CronScheduleRef{
                .handle      = cron_schedule,
                .callback_id = callback_id
            });

            return cron_schedule_id;
        }

        std::size_t RegisterScopedConnection(boost::signals2::scoped_connection connection)
        {
            const auto connection_id = NextId();
            m_signals.emplace(connection_id, std::move(connection));
            return connection_id;
        }

        std::size_t RegisterTimer(int interval, sol::main_protected_function func, bool one_shot)
        {
            if (interval < 10)
            {
                throw sol::error("timer interval must be more than 10ms");
            }

            const auto timer_id    = NextId();
            const auto callback_id = RegisterCallback(std::move(func), one_shot);

            auto timer = std::make_shared<Timer>(
                io,
                interval,
                [w = weak_from_this(), timer_id]()
                {
                    if (auto self = w.lock())
                    {
                        self->TriggerTimer(timer_id);
                    }
                });

            m_timers.emplace(timer_id, TimerRef{
                .handle      = timer,
                .callback_id = callback_id,
                .one_shot    = one_shot
            });

            return timer_id;
        }

        void RemoveCallback(std::size_t callback_id)
        {
            const auto entry = m_callbacks.find(callback_id);

            if (entry == m_callbacks.end())
            {
                BOOST_LOG_TRIVIAL(error) << "plugin[" << plugin_id << "] No callback with id " << callback_id;
                return;
            }

            m_callbacks.erase(entry);
        }

        uWS::App*                                                 app;
        Config&                                                   cfg;
        std::shared_ptr<CurlMulti>                                curl;
        sqlite3*                                                  db;
        std::vector<std::function<void()>>                        destructors;
        boost::asio::io_context&                                  io;
        std::unordered_set<std::shared_ptr<Types::PoHttpServerResponse>> http_responses;
        std::weak_ptr<Rpc::JsonRpc>                               jsonrpc;
        sol::state_view                                           lua;
        int                                                       plugin_id;
        porla::Sessions&                                          sessions;
        boost::asio::thread_pool&                                 sodium_hash_pool;

    private:
        struct CallbackRef
        {
            sol::main_protected_function callback;
            bool                         one_shot;
        };

        struct CronScheduleRef
        {
            std::shared_ptr<CronSchedule> handle;
            std::size_t                   callback_id;
        };

        struct TimerRef
        {
            std::shared_ptr<Timer> handle;
            std::size_t            callback_id;
            bool                   one_shot;
        };

        std::size_t NextId()
        {
            return m_next_id++;
        }

        template <typename Map>
        void CancelEntry(Map& registry, std::size_t id)
        {
            const auto entry = registry.find(id);
            if (entry == registry.end()) { return; }

            const auto callback_id = entry->second.callback_id;
            const auto handle      = entry->second.handle;

            registry.erase(entry);
            m_callbacks.erase(callback_id);

            handle->Cancel();
        }

        void TriggerCronSchedule(std::size_t cron_schedule_id)
        {
            const auto entry = m_cron_schedules.find(cron_schedule_id);

            if (entry == m_cron_schedules.end())
            {
                return;
            }

            const auto callback_id = entry->second.callback_id;
            [[maybe_unused]] const auto keep = entry->second.handle;

            InvokeCallback(entry->second.callback_id);
        }

        void TriggerTimer(std::size_t timer_id)
        {
            const auto entry = m_timers.find(timer_id);

            if (entry == m_timers.end())
            {
                BOOST_LOG_TRIVIAL(error) << "plugin[" << plugin_id << "] Failed to find timer id " << timer_id;
                return;
            }

            const auto callback_id = entry->second.callback_id;
            [[maybe_unused]] const auto keep        = entry->second.handle;

            if (entry->second.one_shot)
            {
                m_timers.erase(entry);
            }

            InvokeCallback(callback_id);
        }

        std::map<std::size_t, CallbackRef>                        m_callbacks;
        std::map<std::size_t, CronScheduleRef>                    m_cron_schedules;
        std::size_t                                               m_next_id = 1;
        std::map<std::size_t, boost::signals2::scoped_connection> m_signals;
        std::map<std::size_t, TimerRef>                           m_timers;
    };
}
