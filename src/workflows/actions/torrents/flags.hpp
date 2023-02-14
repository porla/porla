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
    class Flags : public porla::Workflows::Action
    {
    public:
        explicit Flags(ISession& session);
        ~Flags();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        ISession& m_session;
    };
}
