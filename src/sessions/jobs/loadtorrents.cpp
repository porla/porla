#include "loadtorrents.hpp"

#include <boost/log/trivial.hpp>

#include "../session.hpp"

using porla::Jobs::LoadTorrents;

namespace
{
    static constexpr int  kLoadChunkSize      = 100;
    static constexpr int  kLoadMaxChunkErrors = 8;
    static constexpr auto kLoadRetryBaseDelay = std::chrono::milliseconds(100);
    static constexpr auto kLoadRetryMaxDelay  = std::chrono::milliseconds(5000);
}

LoadTorrents::LoadTorrents(int count, std::function<void()> callback)
    : m_count(count)
    , m_callback(callback)
{
}

porla::Job::Result LoadTorrents::Run(Session& session)
{
    bool more = false;

    try
    {
        more     = session.LoadChunk(m_cursor, kLoadChunkSize, m_loaded);
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

    session.LoadDone(m_loaded, m_failed);

    Complete(session);

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
