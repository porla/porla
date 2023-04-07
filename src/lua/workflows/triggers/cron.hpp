#pragma once

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <sol/sol.hpp>

#include "../trigger.hpp"

namespace porla { class ISession; }

namespace porla::Lua::Workflows::Triggers
{
    struct CronOptions
    {
        std::vector<sol::object> actions;
        std::string              expression;
        boost::asio::io_context& io;
        sol::state&              lua;
        porla::ISession&         session;
    };

    class Cron : public Trigger
    {
    public:
        explicit Cron(const CronOptions& opts);
        ~Cron();

    private:
        void OnTimerExpired(const boost::system::error_code& ec);
        void ScheduleNext();

        CronOptions m_opts;
        boost::asio::deadline_timer m_timer;
        int m_currently_running_workflows;
    };
}
