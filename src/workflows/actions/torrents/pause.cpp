#include "pause.hpp"

#include "../../../json/lttorrentstatus.hpp"
#include "../../../session.hpp"

using porla::Workflows::Actions::Torrents::Pause;

struct Pause::TorrentPauseState
{
    std::shared_ptr<ActionCallback> callback;
};

Pause::Pause(porla::ISession& session)
    : m_session(session)
{
    m_torrent_paused_connection = m_session.OnTorrentPaused([this](auto && s) { OnTorrentPaused(s); });
}

Pause::~Pause()
{
    m_torrent_paused_connection.disconnect();
}

void Pause::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
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

    lt::pause_flags_t flags = {};

    if (params.Input().contains("graceful") && params.Input()["graceful"].get<bool>())
    {
        flags = lt::torrent_handle::graceful_pause;
    }

    th->second.pause(flags);

    auto state = std::make_unique<TorrentPauseState>();
    state->callback = callback;

    m_states.insert({ ts.info_hashes, std::move(state) });
}

void Pause::OnTorrentPaused(const libtorrent::torrent_handle &th)
{
    auto ctx = m_states.find(th.info_hashes());
    if (ctx == m_states.end()) return;

    ctx->second->callback->Complete(true);

    m_states.erase(ctx);
}
