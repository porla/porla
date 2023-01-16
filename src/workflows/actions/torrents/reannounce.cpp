#include "reannounce.hpp"

#include "../../../json/lttorrentstatus.hpp"
#include "../../../session.hpp"

using porla::Workflows::Actions::Torrents::Reannounce;

Reannounce::Reannounce(porla::ISession& session)
    : m_session(session)
{
}

Reannounce::~Reannounce() = default;

void Reannounce::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
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

    th->second.force_reannounce();

    callback->Complete({});
}
