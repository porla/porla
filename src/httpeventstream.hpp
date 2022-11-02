#pragma once

#include <map>
#include <memory>

#include <boost/signals2.hpp>
#include <libtorrent/torrent_status.hpp>

#include "httpcontext.hpp"

namespace porla
{
    class ISession;

    class HttpEventStream
    {
    public:
        explicit HttpEventStream(ISession& session);
        HttpEventStream(const HttpEventStream&) = delete;

        ~HttpEventStream();

        void operator()(std::shared_ptr<HttpContext>);

    private:
        class ContextState;

        void Broadcast(const std::string& name, const std::string& data);
        void OnSessionStats(const std::map<std::string, int64_t>& stats);
        void OnStateUpdate(const std::vector<libtorrent::torrent_status>& torrents);
        void OnTorrentPaused(const libtorrent::torrent_status& status);
        void OnTorrentRemoved(const libtorrent::info_hash_t& hash);
        void OnTorrentResumed(const libtorrent::torrent_status& status);

        ISession& m_session;
        std::vector<std::shared_ptr<ContextState>> m_ctxs;

        boost::signals2::connection m_sessionStatsConnection;
        boost::signals2::connection m_stateUpdateConnection;
        boost::signals2::connection m_torrentPausedConnection;
        boost::signals2::connection m_torrentRemovedConnection;
        boost::signals2::connection m_torrentResumedConnection;
    };
}
