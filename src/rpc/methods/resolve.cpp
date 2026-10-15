#include "resolve.hpp"

#include "../../sessions/session.hpp"
#include "../../sessions/sessions.hpp"
#include "../../sessions/torrent.hpp"

namespace porla::Rpc::Methods
{
    std::optional<Data::Models::Sessions::Session> FindSession(
        porla::Sessions& sessions,
        std::optional<int> session_id,
        const ResponseWriterHandle& cb)
    {
        const auto record = sessions.Resolve(session_id);

        if (!record)
        {
            cb->Error(-1, "Session not found");
            return std::nullopt;
        }

        return record;
    }

    std::optional<ResolvedSession> ResolveSession(
        porla::Sessions& sessions,
        std::optional<int> session_id,
        const ResponseWriterHandle& cb)
    {
        auto record = sessions.Resolve(session_id);

        if (!record)
        {
            cb->Error(-1, "Session not found");
            return std::nullopt;
        }

        auto session_state = sessions.Get(record->id);

        if (session_state == nullptr)
        {
            cb->Error(-2, "Session not loaded");
            return std::nullopt;
        }

        return ResolvedSession{
            .record  = record.value(),
            .session = session_state
        };
    }

    std::optional<ResolvedTorrent> ResolveTorrent(
        porla::Sessions& sessions,
        std::optional<int> session_id,
        const lt::info_hash_t& info_hash,
        const ResponseWriterHandle& cb)
    {
        auto resolved_session = ResolveSession(sessions, session_id, cb);

        if (!resolved_session)
        {
            return std::nullopt;
        }

        const auto* torrent = resolved_session->session->Find(info_hash);

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
            std::move(resolved_session->session),
            torrent
        };
    }
}
