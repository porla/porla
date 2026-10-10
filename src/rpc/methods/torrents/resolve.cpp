#include "resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

namespace porla::Rpc::Methods::Torrents
{
    std::shared_ptr<porla::Session> ResolveSession(
        porla::Sessions& sessions,
        std::optional<int> session_id,
        const ResponseWriterHandle& cb)
    {
        const auto id = sessions.ResolveId(session_id);

        if (!id)
        {
            cb->Error(-1, "Session not found");
            return nullptr;
        }

        auto session_state = sessions.Get(*id);

        if (session_state == nullptr)
        {
            cb->Error(-2, "Session not loaded");
            return nullptr;
        }

        return session_state;
    }

    std::optional<ResolvedTorrent> ResolveTorrent(
        porla::Sessions& sessions,
        std::optional<int> session_id,
        const lt::info_hash_t& info_hash,
        const ResponseWriterHandle& cb)
    {
        auto session_state = ResolveSession(sessions, session_id, cb);

        if (session_state == nullptr)
        {
            return std::nullopt;
        }

        const auto* torrent = session_state->Find(info_hash);

        if (torrent == nullptr)
        {
            cb->Error(-3, "Torrent not found in session");
            return std::nullopt;
        }

        if (!torrent->status.handle.is_valid())
        {
            cb->Error(-4, "Torrent not valid");
            return std::nullopt;
        }

        return ResolvedTorrent{
            std::move(session_state),
            torrent
        };
    }
}
