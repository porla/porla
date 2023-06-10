#pragma once

#include <boost/asio.hpp>
#include <sol/sol.hpp>

namespace porla::Lua::UserTypes
{
    class Cron
    {
    public:
        static void Register(sol::state& lua);

        explicit Cron(boost::asio::io_context& io, sol::table args);
        ~Cron();

    private:
        void OnTimerExpired(const boost::system::error_code& ec);
        void ScheduleNext();

        boost::asio::io_context& m_io;
        sol::table m_args;
        boost::asio::deadline_timer m_timer;
    };
}
