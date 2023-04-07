#include "cron.hpp"

#include <boost/log/trivial.hpp>
#include <croncpp.hpp>

#include "../workflowrunner.hpp"
#include "../../usertypes/torrent.hpp"
#include "../../../query/pql.hpp"
#include "../../../session.hpp"

using porla::Lua::UserTypes::Torrent;
using porla::Lua::Workflows::Triggers::Cron;
using porla::Lua::Workflows::Triggers::CronOptions;
using porla::Lua::Workflows::WorkflowRunnerOptions;
using porla::Query::PQL;
using porla::Session;

Cron::Cron(const CronOptions &opts)
    : m_opts(opts)
    , m_timer(opts.io)
    , m_currently_running_workflows(0)
{
    ScheduleNext();
}

Cron::~Cron() = default;

void Cron::OnTimerExpired(const boost::system::error_code &ec)
{
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Workflow::Cron trigger error: " << ec.message();
        return;
    }

    if (m_currently_running_workflows > 0)
    {
        BOOST_LOG_TRIVIAL(warning) << "(triggers/cron) Unfinished workflows - consider loosening the schedule";
    }

    const WorkflowRunnerOptions runner_opts{
        .io      = m_opts.io,
        .lua     = m_opts.lua,
        .session = m_opts.session,
    };

    int launched_workflows = 0;

    std::unique_ptr<PQL::Filter> filter;

    if (!m_opts.query.empty())
    {
        filter = PQL::Parse(m_opts.query);
    }

    for (const auto& [ hash, torrent ] : m_opts.session.Torrents())
    {
        if (filter && !filter->Includes(torrent.status())) continue;

        sol::table ctx           = m_opts.lua.create_table();
        ctx["actions"]           = std::vector<sol::object>();
        ctx["lt:torrent_handle"] = torrent;
        ctx["torrent"]           = Torrent{torrent};

        auto workflow = std::make_shared<WorkflowRunner>(runner_opts, ctx, m_opts.actions);

        boost::signals2::connection connection;
        connection = workflow->OnFinished(
            [&, connection]()
            {
                m_currently_running_workflows--;

                BOOST_LOG_TRIVIAL(info)
                    << "(triggers/cron) Workflow finished (" << m_currently_running_workflows << " still running)";
            });

        boost::asio::post(
            m_opts.io,
            [workflow]() { workflow->Run(); });

        launched_workflows++;
        m_currently_running_workflows++;
    }

    BOOST_LOG_TRIVIAL(info) << "(triggers/cron) Launched " << launched_workflows << " workflow(s)";

    // Immediately schedule next cron invocation.
    ScheduleNext();
}

void Cron::ScheduleNext()
{
    cron::cronexpr expression;

    try
    {
        expression = cron::make_cron(m_opts.expression);
    }
    catch (const cron::bad_cronexpr& err)
    {
        BOOST_LOG_TRIVIAL(error) << "(triggers/cron) Failed to parse cron expression: " << err.what();
        return;
    }

    const std::time_t now = std::time(nullptr);
    const std::time_t next = cron::cron_next(expression, now);

    const auto seconds = next - now;

    BOOST_LOG_TRIVIAL(info) << "(triggers/cron) Next invocation in " << seconds << " seconds";

    m_timer.expires_from_now(boost::posix_time::seconds(seconds));
    m_timer.async_wait([&](const boost::system::error_code& ec) { OnTimerExpired(ec); });
}
