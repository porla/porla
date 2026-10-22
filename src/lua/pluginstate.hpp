#pragma once

#include <map>
#include <memory>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/signals2.hpp>
#include <sol/sol.hpp>
#include <uWebSockets/App.h>

#include "../plugins.hpp"
#include "../timer.hpp"

#include "types/pohttpserverresponse.hpp"

namespace porla
{
    class Config;
    class CurlMulti;
    class Events;
    class KeyValue;
    class Presets;
    class Sessions;
    class Torrents;
}

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla::Lua
{
    struct LuaState : public std::enable_shared_from_this<LuaState>
    {
        static constexpr const char* RegistryKey = "state";

        static std::weak_ptr<LuaState> WeakFrom(sol::this_state ts)
        {
            return sol::state_view(ts).registry()[RegistryKey].get<std::weak_ptr<LuaState>>();
        }

        static std::shared_ptr<LuaState> From(sol::this_state ts)
        {
            return WeakFrom(ts).lock();
        }

        static void Attach(sol::state_view lua, const std::shared_ptr<LuaState>& state)
        {
            lua.registry()[RegistryKey] = std::weak_ptr(state);
        }

        explicit LuaState(const porla::PluginsOptions& opts, int plugin_id, sol::state_view lua)
            : app(opts.http_server)
            , cfg(opts.cfg)
            , curl(opts.curl_multi)
            , events(opts.events)
            , io(opts.io)
            , jsonrpc(opts.jsonrpc)
            , kv(opts.kv)
            , presets(opts.presets)
            , lua(lua)
            , plugin_id(plugin_id)
            , sessions(opts.sessions)
            , torrents(opts.torrents)
            , sodium_hash_pool(opts.hash_pool)
        {
        }

        ~LuaState()
        {
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
            const auto entry = m_timers.find(timer_id);
            if (entry == m_timers.end()) { return; }

            const auto callback_id = entry->second.callback_id;
            const auto handle      = entry->second.handle;

            m_timers.erase(entry);
            m_callbacks.erase(callback_id);

            handle->Cancel();
        }

        template <typename... Args>
        bool InvokeCallback(std::size_t callback_id, Args&&... args)
        {
            const auto entry = m_callbacks.find(callback_id);

            if (entry == m_callbacks.end())
            {
                BOOST_LOG_TRIVIAL(error) << "plugin[" << plugin_id << "] No callback with id " << callback_id;
                return false;
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
                return false;
            }

            return true;
        }

        boost::asio::any_io_executor IoExecutor() const { return io.get_executor(); }

        std::size_t RegisterCallback(sol::main_protected_function func, bool one_shot)
        {
            const auto id = NextId();
            m_callbacks[id] = CallbackRef{.callback = std::move(func), .one_shot = one_shot};
            return id;
        }

        std::size_t RegisterCron(const std::string& expression, sol::main_protected_function func)
        {
            const auto timer_id = NextId();
            auto timer = Timer::Cron(io, expression, Trigger(timer_id));
            return AddTimer(timer_id, std::move(timer), std::move(func), false);
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

            const auto timer_id = NextId();
            auto timer = Timer::Interval(io, std::chrono::milliseconds(interval), Trigger(timer_id));
            return AddTimer(timer_id, std::move(timer), std::move(func), one_shot);
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
        std::vector<std::function<void()>>                        destructors;
        porla::Events&                                            events;
        boost::asio::io_context&                                  io;
        std::unordered_set<std::shared_ptr<Types::PoHttpServerResponse>> http_responses;
        std::weak_ptr<Rpc::JsonRpc>                               jsonrpc;
        porla::KeyValue&                                          kv;
        porla::Presets&                                           presets;
        sol::state_view                                           lua;
        int                                                       plugin_id;
        porla::Sessions&                                          sessions;
        porla::Torrents&                                          torrents;
        boost::asio::thread_pool&                                 sodium_hash_pool;

    private:
        struct CallbackRef
        {
            sol::main_protected_function callback;
            bool                         one_shot;
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

        std::size_t AddTimer(std::size_t timer_id, std::shared_ptr<Timer> timer, sol::main_protected_function func, bool one_shot)
        {
            m_timers.emplace(timer_id, TimerRef{
                .handle      = std::move(timer),
                .callback_id = RegisterCallback(std::move(func), one_shot),
                .one_shot    = one_shot
            });

            return timer_id;
        }

        std::function<void()> Trigger(std::size_t timer_id)
        {
            return [w = weak_from_this(), timer_id]()
            {
                if (auto self = w.lock())
                {
                    self->TriggerTimer(timer_id);
                }
            };
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
        std::size_t                                               m_next_id = 1;
        std::map<std::size_t, boost::signals2::scoped_connection> m_signals;
        std::map<std::size_t, TimerRef>                           m_timers;
    };
}
