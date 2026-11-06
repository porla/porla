#include "loadtorrents.hpp"

#include "../scheduler.hpp"
#include "../session.hpp"
#include "../sessionevent.hpp"

#include "../../events.hpp"

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
        PORLA_LOG(session.Logger(), info, "Loading {count} torrent(s) from storage", m_count);
    }

    try
    {
        session.ReadAlerts();
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_SEV(session.Logger(), Log::error)
            << Log::Property("error", e.what())
            << "Failed to read session alerts during load";
    }
    
    bool more = false;

    try
    {
        more = AddTorrentParams::Next(
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

            BOOST_LOG_SEV(session.Logger(), Log::warning)
                << Log::Property("error", e.what())
                << "Chunk failed at " << m_loaded
                << " of " << m_count << " (attempt " << m_errors << " of "
                << kLoadMaxChunkErrors << ", retrying in " << delay.count() << "ms)";

            return Again(delay);
        }

        BOOST_LOG_SEV(session.Logger(), Log::error)
            << Log::Property("error", e.what())
            << "Failed to load torrents after "
            << m_loaded << " of " << m_count;

        m_failed = true;
    }

    if (more)
    {
        m_chunks++;

        if (m_chunks % 10 == 0)
        {
            PORLA_LOG(session.Logger(), info, "{added} torrents (of {total}) added", m_loaded, m_count);
        }

        return Again();
    }

    if (m_count > 0)
    {
        if (m_failed)
        {
            BOOST_LOG_SEV(session.Logger(), Log::error)
                << "Incomplete load - " << m_loaded << " of " << m_count
                << " torrent(s) were added. The remaining "
                << m_count - m_loaded << " are still in the database but not in the session. "
                << "Do not re-add them; restart Porla once the database is healthy.";
        }
        else
        {
            PORLA_LOG(session.Logger(), info,
                "Added {added} (of {total}) torrent(s) to the session",
                m_loaded,
                m_count);
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
        BOOST_LOG_SEV(session.Logger(), Log::error)
            << Log::Property("error", e.what())
            << "Load completion callback failed";
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
