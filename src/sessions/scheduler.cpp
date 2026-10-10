#include "scheduler.hpp"

#include <boost/asio/steady_timer.hpp>
#include <boost/log/trivial.hpp>

#include "jobs/job.hpp"

using porla::Job;
using porla::Scheduler;

namespace
{
    static constexpr auto kSlowJobThreshold = std::chrono::milliseconds(20);
}

struct Scheduler::Entry
{
    Kind                      kind;
    std::type_index           type;
    std::chrono::milliseconds interval;
    std::unique_ptr<Job>      job;
    boost::asio::steady_timer timer;
    bool                      scheduled = false;
    bool                      running = false;
    bool                      triggered = false;
};

struct Scheduler::State
{
    boost::asio::io_context&            io;
    Session&                            session;
    std::vector<std::shared_ptr<Entry>> entries;
    int                                 suspends   = 0;
    bool                                stopped = false;
};

Scheduler::Scheduler(boost::asio::io_context& io, Session& session)
    : m_state(std::make_shared<State>(State{ .io = io, .session = session }))
{
}

Scheduler::~Scheduler()
{
    Stop();
}

void Scheduler::Every(std::chrono::milliseconds interval, std::unique_ptr<Job> job)
{
    Schedule(m_state, Add(*m_state, Kind::Periodic, interval, std::move(job)), interval);
}

void Scheduler::Start(std::unique_ptr<Job> job)
{
    Schedule(m_state, Add(*m_state, Kind::Once, {}, std::move(job)), {});
}

void Scheduler::Register(std::unique_ptr<Job> job)
{
    Add(*m_state, Kind::Manual, {}, std::move(job));
}

void Scheduler::Trigger(std::type_index type)
{
    for (const auto& entry : m_state->entries)
    {
        if (entry->kind != Kind::Manual || entry->type != type)
        {
            continue;
        }

        entry->triggered = true;
    
        if (m_state->suspends == 0 && !entry->scheduled && !entry->running)
        {
            Schedule(m_state, entry, {});
        }

        return;
    }

    BOOST_LOG_TRIVIAL(warning) << "Job of type " << type.name() << " not registered";
}

void Scheduler::Suspend()
{
    m_state->suspends++;
}

void Scheduler::Resume()
{
    m_state->suspends = std::max(0, m_state->suspends);

    if (m_state->suspends == 0)
    {
        return;
    }

    m_state->suspends--;

    if (m_state->suspends > 0)
    {
        return;
    }

    for (const auto& entry : m_state->entries)
    {
        if (entry->kind == Kind::Manual
            && entry->triggered
            && !entry->scheduled
            && !entry->running)
        {
            Schedule(m_state, entry, {});
        }
    }
}

void Scheduler::Stop()
{
    if (m_state->stopped)
    {
        return;
    }

    m_state->stopped = true;

    // take a copy of the entries since
    // Stopped() might start or register jobs
    const auto entries = m_state->entries;

    for (const auto& entry : entries)
    {
        entry->timer.cancel();

        try
        {
            entry->job->Stopped(m_state->session);
        }
        catch(const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error)
                << "Job " << entry->job->Name() << " failed to stop: " << e.what();
        }
        
    }
}

std::shared_ptr<Scheduler::Entry> Scheduler::Add(
    State& state,
    Kind kind,
    std::chrono::milliseconds interval,
    std::unique_ptr<Job> job)
{
    const Job&            ref  = *job;
    const std::type_index type = typeid(ref);

    return state.entries.emplace_back(
        std::make_shared<Entry>(Entry{
        .kind     = kind,
        .type     = type,
        .interval = interval,
        .job      = std::move(job),
        .timer    = boost::asio::steady_timer(state.io)
    }));
}

void Scheduler::Schedule(
    const std::shared_ptr<State>& state,
    const std::shared_ptr<Entry>& entry,
    std::chrono::milliseconds delay)
{
    if (state->stopped)
    {
        return;
    }

    entry->scheduled = true;
    entry->timer.expires_after(delay);
    entry->timer.async_wait(
        [weak_state = std::weak_ptr(state), weak_entry = std::weak_ptr(entry)](const boost::system::error_code& ec)
        {
            const auto s = weak_state.lock();
            const auto e = weak_entry.lock();

            if (ec || !s || !e || s->stopped)
            {
                return;
            }

            Run(s, e);
        });
}

void Scheduler::Run(
    const std::shared_ptr<State>& state,
    const std::shared_ptr<Entry>& entry)
{
    entry->scheduled = false;
    entry->running   = true;
    entry->triggered = false;

    Job::Result result;

    const auto started = std::chrono::steady_clock::now();

    try
    {
        result = entry->job->Run(state->session);
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << "Job " << entry->job->Name() << " failed: " << e.what();
    }

    entry->running = false;

    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - started);

    if (elapsed > kSlowJobThreshold)
    {
        BOOST_LOG_TRIVIAL(warning) << "Job " << entry->job->Name() << " took " << elapsed.count() << " ms";
    }

    if (state->stopped)
    {
        return;
    }

    if (result.again)
    {
        Schedule(state, entry, result.delay);
        return;
    }

    switch (entry->kind)
    {
    case Kind::Periodic:
        Schedule(state, entry, entry->interval);
        break;

    case Kind::Once:
        std::erase(state->entries, entry);
        break;

    case Kind::Manual:
        // triggered while it was running
        if (entry->triggered && state->suspends == 0)
        {
            Schedule(state, entry, {});
        }

        break;
    }
}
