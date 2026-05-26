#pragma once

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <cmrc/cmrc.hpp>
#include <sol/sol.hpp>

namespace porla
{
    class CurlMulti;
}

namespace porla::Lua
{
    class Host
    {
    public:
        explicit Host(boost::asio::io_context& io, CurlMulti& cm);
        ~Host();

        void Run(const boost::program_options::variables_map& args);
        void Stop(int timeout_ms, std::function<void()> callback);

        template<typename... Args>
        void SpawnCoroutine(sol::protected_function fn, Args&&... args)
        {
            sol::thread th = sol::thread::create(m_lua);
            const auto result = sol::coroutine(th.state(), fn)(std::forward<Args>(args)...);
            SpawnCoroutineInternal(std::move(th), result);
        }

    private:
        void PollShutdown(lua_State* L, std::function<void()> callback);
        void ScheduleCoroutineCleanup();
        void SpawnCoroutineInternal(sol::thread th, const sol::protected_function_result& result);

        struct CoroutineState
        {
            sol::thread thread;
        };

        porla::CurlMulti& m_cm;
        boost::asio::io_context& m_io;
        sol::state m_lua;
        sol::table m_bootstrap_table;
        std::vector<CoroutineState> m_active_coroutines;
        boost::asio::steady_timer m_active_coroutines_cleanup_timer;
        boost::asio::steady_timer m_shutdown_deadline_timer;
        boost::asio::steady_timer m_shutdown_poll_timer;
    };
}