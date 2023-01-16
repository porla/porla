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
    class Reannounce : public porla::Workflows::Action
    {
    public:
        explicit Reannounce(ISession& session);
        ~Reannounce();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        ISession& m_session;
    };
}
