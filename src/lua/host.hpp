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

    private:
        void PollShutdown(lua_State* L, std::function<void()> callback);
        void ScheduleCoroutineCleanup();

        porla::CurlMulti& m_cm;
        boost::asio::io_context& m_io;
        sol::state m_lua;
        sol::table m_bootstrap_table;
        std::vector<sol::thread> m_active_coroutines;
        boost::asio::steady_timer m_active_coroutines_cleanup_timer;
        boost::asio::steady_timer m_shutdown_deadline_timer;
        boost::asio::steady_timer m_shutdown_poll_timer;
    };
}