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
    class Move : public porla::Workflows::Action
    {
    public:
        explicit Move(ISession& session);
        ~Move();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        void OnStorageMoved(const libtorrent::torrent_handle& th);

        struct TorrentMoveState;

        boost::signals2::connection m_storage_moved_connection;

        ISession& m_session;
        std::map<libtorrent::info_hash_t, std::unique_ptr<TorrentMoveState>> m_states;
    };
}
