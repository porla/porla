#include "loadtorrents.hpp"

#include <boost/log/trivial.hpp>

#include "../session.hpp"

using porla::Data::Models::AddTorrentParams;
using porla::Jobs::LoadTorrents;

namespace
{
    static constexpr int  kLoadChunkSize      = 100;
    static constexpr int  kLoadMaxChunkErrors = 8;
    static constexpr auto kLoadRetryBaseDelay = std::chrono::milliseconds(100);
    static constexpr auto kLoadRetryMaxDelay  = std::chrono::milliseconds(5000);
}

LoadTorrents::LoadTorrents(sqlite3* db, int count, std::function<void(const Output&)> callback)
    : m_db(db)
    , m_count(count)
    , m_callback(std::move(callback))
{
}

porla::Job::Result LoadTorrents::Run(Session& session)
{
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

    Complete(session, false);

    return Done();
}

void LoadTorrents::Stopped(Session& session)
{
    Complete(session, true);
}

void LoadTorrents::Complete(const Session& session, bool stopped)
{
    auto callback = std::exchange(m_callback, nullptr);

    if (!callback)
    {
        return;
    }

    try
    {
        callback(Output{
            .loaded  = m_loaded,
            .failed  = m_failed,
            .stopped = stopped
        });
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error)
            << session.Log() << "Load completion callback failed: " << e.what();
    }
}
