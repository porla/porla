#include "remove.hpp"

#include "../../../json/lttorrentstatus.hpp"
#include "../../../session.hpp"

using porla::Workflows::Actions::Torrents::Remove;

struct Remove::TorrentRemoveState
{
    std::shared_ptr<ActionCallback> callback;
};

Remove::Remove(porla::ISession& session)
    : m_session(session)
{
    m_torrent_removed_connection = m_session.OnTorrentRemoved([this](auto && s) { OnTorrentRemoved(s); });
}

Remove::~Remove()
{
    m_torrent_removed_connection.disconnect();
}

void Remove::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    const auto& torrent = params.Render("torrent", true);
    bool remove_files = false;

    if (params.Input().contains("remove_files"))
    {
        remove_files = params.Input()["remove_files"];
    }

    if (torrent == nullptr)
    {
        // WARN
        return;
    }

    const lt::torrent_status& ts = torrent;
    const auto& th = m_session.Torrents().find(ts.info_hashes);

    if (th == m_session.Torrents().end())
    {
        return;
    }

    m_session.Remove(ts.info_hashes, remove_files);

    auto state = std::make_unique<TorrentRemoveState>();
    state->callback = callback;

    m_states.insert({ ts.info_hashes, std::move(state) });
}

void Remove::OnTorrentRemoved(const libtorrent::info_hash_t& hash)
{
    auto ctx = m_states.find(hash);
    if (ctx == m_states.end()) return;

    ctx->second->callback->Complete(true);

    m_states.erase(ctx);
}
