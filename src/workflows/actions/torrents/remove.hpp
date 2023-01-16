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
    class Remove : public porla::Workflows::Action
    {
    public:
        explicit Remove(ISession& session);
        ~Remove();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        void OnTorrentRemoved(const libtorrent::info_hash_t& hash);

        struct TorrentRemoveState;

        boost::signals2::connection m_torrent_removed_connection;

        ISession& m_session;
        std::map<libtorrent::info_hash_t, std::unique_ptr<TorrentRemoveState>> m_states;
    };
}
