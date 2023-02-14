#pragma once

#include <map>
#include <memory>

#include <boost/signals2.hpp>
#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_handle.hpp>

#include "../../action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Workflows::Actions::Torrents
{
    class Pause : public porla::Workflows::Action
    {
    public:
        explicit Pause(ISession& session);
        ~Pause();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        void OnTorrentPaused(const libtorrent::torrent_handle& th);

        struct TorrentPauseState;

        boost::signals2::connection m_torrent_paused_connection;

        ISession& m_session;
        std::map<libtorrent::info_hash_t, std::unique_ptr<TorrentPauseState>> m_states;
    };
}
