#include "loadtorrents.hpp"

#include <boost/log/trivial.hpp>

#include "../../events.hpp"
#include "../scheduler.hpp"
#include "../session.hpp"
#include "../sessionevent.hpp"

using porla::Data::Models::AddTorrentParams;
using porla::Jobs::LoadTorrents;

namespace
{
    static constexpr int  kLoadChunkSize      = 100;
    static constexpr int  kLoadMaxChunkErrors = 8;
    static constexpr auto kLoadRetryBaseDelay = std::chrono::milliseconds(100);
    static constexpr auto kLoadRetryMaxDelay  = std::chrono::milliseconds(5000);
}

LoadTorrents::LoadTorrents(sqlite3* db, Events& events, Scheduler& scheduler, std::function<void()> callback)
    : m_db(db)
    , m_events(events)
    , m_scheduler(scheduler)
    , m_callback(std::move(callback))
{
    m_scheduler.Suspend();
}

porla::Job::Result LoadTorrents::Run(Session& session)
{
    if (m_count < 0)
    {
        m_count = AddTorrentParams::Count(m_db, session.Id());

        BOOST_LOG_TRIVIAL(info)
            << session.Log() << "Loading " << m_count << " torrent(s) from storage";
    }

    try
    {
        session.ReadAlerts();
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << session.Log() << "Failed to read alerts during load: " << e.what();
    }
    
    bool more = false;

    try
    {
        more     = AddTorrentParams::Next(
            m_db,
            session.Id(),
            m_cursor,
            kLoadChunkSize,
            [&](lt::add_torrent_params& params)
            {
                if (session.Add(params, Torrent::State::Loading))
                {
                    m_loaded++;
                }
            });

        m_errors = 0;
    }
    catch(const std::exception& e)
    {
        m_errors++;

        if (m_errors <= kLoadMaxChunkErrors)
        {
            const auto delay = std::min(
                kLoadRetryMaxDelay,
                kLoadRetryBaseDelay * (1 << (m_errors - 1)));

            BOOST_LOG_TRIVIAL(warning)
                << session.Log() << "Chunk failed at " << m_loaded
                << " of " << m_count << " (attempt " << m_errors << " of "
                << kLoadMaxChunkErrors << ", retrying in " << delay.count()
                << "ms): " << e.what();

            return Again(delay);
        }

        BOOST_LOG_TRIVIAL(error)
            << session.Log() << "Failed to load torrents after "
            << m_loaded << " of " << m_count << ": " << e.what();

        m_failed = true;
    }

    if (more)
    {
        m_chunks++;

        if (m_chunks % 10 == 0)
        {
            BOOST_LOG_TRIVIAL(info)
                << session.Log() << m_loaded << " torrents (of "
                << m_count << ") added";
        }

        return Again();
    }

    if (m_count > 0)
    {
        if (m_failed)
        {
            BOOST_LOG_TRIVIAL(error)
                << session.Log() << "Incomplete load - "
                << m_loaded << " of " << m_count
                << " torrent(s) were added. The remaining "
                << m_count - m_loaded << " are still in the database but not in the session. "
                << "Do not re-add them; restart Porla once the database is healthy.";
        }
        else
        {
            BOOST_LOG_TRIVIAL(info)
                << session.Log() << "Added " << m_loaded
                << " (of " << m_count << ") torrent(s) to the session";
        }
    }

    Finish(session);

    return Done();
}

void LoadTorrents::Stopped(Session& session)
{
    Complete(session);
}

void LoadTorrents::Complete(const Session& session)
{
    auto callback = std::exchange(m_callback, nullptr);

    if (!callback)
    {
        return;
    }

    try
    {
        callback();
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << session.Log() << "Load completion callback failed: " << e.what();
    }
}

void LoadTorrents::Finish(Session& session)
{
    if (m_loaded > 0)
    {
        session.ReconcileTorrents();
    }

    SessionEvent loaded("session.loaded", {
        { "torrents", m_loaded },
        { "failed",   m_failed }
    });

    loaded.session_id = session.Id();
    loaded.session    = session.weak_from_this();

    m_events.Publish(std::move(loaded));

    // post torrent updates immediately to reduce the gap where torrents have no status
    // without this it would take one tick of the post updates timer to receive full status
    session.Libtorrent().post_torrent_updates();

    m_scheduler.Resume();

    Complete(session);
}
