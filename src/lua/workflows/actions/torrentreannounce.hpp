#pragma once

#include <map>
#include <memory>

#include <libtorrent/info_hash.hpp>

#include "../action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Workflows::Actions::Torrents
{
    class Reannounce : public porla::Workflows::Action
    {
    public:
        explicit Reannounce(ISession& session);
        ~Reannounce();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        void OnTorrentTrackerError(const libtorrent::tracker_error_alert* al);
        void OnTorrentTrackerReply(const libtorrent::torrent_handle& th);

        struct TorrentReannounceState;

        boost::signals2::connection m_torrent_tracker_error_connection;
        boost::signals2::connection m_torrent_tracker_reply_connection;

        ISession& m_session;
        std::map<libtorrent::info_hash_t, std::unique_ptr<TorrentReannounceState>> m_states;
    };
}
