#include "interval.hpp"

#include <boost/log/trivial.hpp>

#include "../workflowrunner.hpp"
#include "../../usertypes/torrent.hpp"
#include "../../../session.hpp"

using porla::Lua::UserTypes::Torrent;
using porla::Lua::Workflows::Triggers::Interval;
using porla::Lua::Workflows::Triggers::IntervalOptions;
using porla::Lua::Workflows::WorkflowRunnerOptions;
using porla::Session;

Interval::Interval(const IntervalOptions &opts)
    : m_opts(opts)
    , m_timer(opts.io)
    , m_currently_running_workflows(0)
{
    m_timer.expires_from_now(boost::posix_time::milliseconds(opts.interval));
    m_timer.async_wait([&](const boost::system::error_code& ec) { OnTimerExpired(ec); });
}

Interval::~Interval() = default;

void Interval::OnTimerExpired(const boost::system::error_code &ec)
{
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Workflow::Interval timer error: " << ec.message();
        return;
    }

    const WorkflowRunnerOptions runner_opts{
        .io      = m_opts.io,
        .lua     = m_opts.lua,
        .session = m_opts.session,
    };

    for (const auto& [ hash, torrent ] : m_opts.session.Torrents())
    {
        sol::table ctx           = m_opts.lua.create_table();
        ctx["actions"]           = std::vector<sol::object>();
        ctx["lt:torrent_handle"] = torrent;
        ctx["torrent"]           = Torrent{torrent};

        auto workflow = std::make_shared<WorkflowRunner>(runner_opts, ctx, m_opts.actions);

        m_on_workflow_finished.push_back(
            workflow->OnFinished(
                [&]()
                {
                    m_currently_running_workflows--;

                    if (m_currently_running_workflows == 0)
                    {
                        BOOST_LOG_TRIVIAL(info) << "All workflows for this interval have finished - rescheduling";

                        m_on_workflow_finished.clear();

                        m_timer.expires_from_now(boost::posix_time::milliseconds(m_opts.interval));
                        m_timer.async_wait([&](const boost::system::error_code& ec) { OnTimerExpired(ec); });
                    }
                }));

        boost::asio::post(
            m_opts.io,
            [workflow]() { workflow->Run(); });

        m_currently_running_workflows++;
    }
}
