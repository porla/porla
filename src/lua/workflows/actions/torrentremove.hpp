#pragma once

#include <map>
#include <memory>

#include <libtorrent/info_hash.hpp>

#include "../action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Lua::Workflows::Actions
{
    struct TorrentRemoveOptions
    {
        porla::ISession& session;
        bool             remove_files;
    };

    class TorrentRemove : public Action
    {
    public:
        explicit TorrentRemove(TorrentRemoveOptions opts);
        ~TorrentRemove();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        void OnTorrentRemoved(const libtorrent::info_hash_t& hash);

        struct TorrentRemoveState;

        TorrentRemoveOptions m_opts;
        std::unique_ptr<TorrentRemoveState> m_state;
    };
}
