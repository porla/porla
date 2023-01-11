#include "move.hpp"

#include "../../../json/lttorrentstatus.hpp"
#include "../../../session.hpp"

using porla::Workflows::Actions::Torrents::Move;

struct Move::TorrentMoveState
{
    std::shared_ptr<ActionCallback> callback;
};

Move::Move(porla::ISession& session)
    : m_session(session)
{
    m_storage_moved_connection = m_session.OnStorageMoved([this](auto && s) { OnStorageMoved(s); });
}

Move::~Move()
{
    m_storage_moved_connection.disconnect();
}

void Move::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    const auto& path    = params.Input()["path"].get<std::string>();
    const auto& torrent = params.Render("torrent", true);

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

    th->second.move_storage(path);

    auto state = std::make_unique<TorrentMoveState>();
    state->callback = callback;

    m_states.insert({ ts.info_hashes, std::move(state) });
}

void Move::OnStorageMoved(const libtorrent::torrent_handle& th)
{
    auto ctx = m_states.find(th.info_hashes());
    if (ctx == m_states.end()) return;

    ctx->second->callback->Complete(true);

    m_states.erase(ctx);
}
