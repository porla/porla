#pragma once

#include <map>

#include <boost/signals2.hpp>
#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_status.hpp>

#include "action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Actions
{
    class Move : public Action
    {
    public:
        explicit Move(ISession& session);
        ~Move() override;

        void Invoke(const libtorrent::info_hash_t& hash, const std::vector<std::string>& args, const std::shared_ptr<ActionCallback>& callback) override;

    private:
        void OnStorageMoved(const libtorrent::torrent_handle& th);

        struct MoveContext;

        boost::signals2::connection m_storageMovedConnection;
        boost::signals2::connection m_storageMovedFailedConnection;

        ISession& m_session;
        std::map<libtorrent::info_hash_t, std::unique_ptr<MoveContext>> m_state;
    };
}
