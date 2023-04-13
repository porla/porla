#pragma once

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <sol/sol.hpp>

#include "../trigger.hpp"

namespace porla { class ISession; }

namespace porla::Lua::Workflows::Triggers
{
    struct IntervalOptions
    {
        std::vector<sol::object> actions;
        sol::object              filter;
        int                      interval;
        boost::asio::io_context& io;
        sol::state&              lua;
        porla::ISession&         session;
    };

    class Interval : public Trigger
    {
    public:
        explicit Interval(const IntervalOptions& opts);
        ~Interval();

    private:
        void OnTimerExpired(const boost::system::error_code& ec);

        IntervalOptions m_opts;
        boost::asio::deadline_timer m_timer;
        std::vector<boost::signals2::connection> m_on_workflow_finished;
        int m_currently_running_workflows;
    };
}
