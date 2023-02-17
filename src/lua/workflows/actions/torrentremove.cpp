#include "torrentremove.hpp"

#include "../../../session.hpp"

using porla::Lua::Workflows::Actions::TorrentRemove;
using porla::Lua::Workflows::Actions::TorrentRemoveOptions;

struct TorrentRemove::TorrentRemoveState
{
    std::shared_ptr<ActionCallback> callback;
    boost::signals2::connection     torrent_removed_connection;
};

TorrentRemove::TorrentRemove(TorrentRemoveOptions opts)
    : m_opts(opts)
{
    m_state = std::make_unique<TorrentRemoveState>();
    m_state->torrent_removed_connection = opts.session.OnTorrentRemoved([this](auto && s) { OnTorrentRemoved(s); });
}

TorrentRemove::~TorrentRemove()
{
    m_state->torrent_removed_connection.disconnect();
}

void TorrentRemove::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    m_state->callback = callback;

    const lt::torrent_handle& th = params.context["lt:torrent_handle"];
    m_opts.session.Remove(th.info_hashes(), m_opts.remove_files);
}

void TorrentRemove::OnTorrentRemoved(const libtorrent::info_hash_t& hash)
{
    m_state->callback->Complete();
    m_state->callback = nullptr;
}
