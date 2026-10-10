#include "torrentspropertiesset.hpp"

#include "../../../data/models/addtorrentparams.hpp"
#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"
#include "../../../torrents.hpp"
#include "../../../torrentclientdata.hpp"
#include "../../../utils/limits.hpp"

using porla::Rpc::Methods::Torrents::TorrentsPropertiesSet;
using porla::Rpc::Methods::Torrents::TorrentsPropertiesSetReq;
using porla::Rpc::Methods::Torrents::TorrentsPropertiesSetRes;

TorrentsPropertiesSet::TorrentsPropertiesSet(sqlite3* db, porla::Sessions& sessions, porla::Torrents& torrents)
    : m_db(db)
    , m_sessions(sessions)
    , m_torrents(torrents)
{
}

void TorrentsPropertiesSet::Execute(const TorrentsPropertiesSetReq& req, ResponseWriterHandle cb)
{
    const auto session = req.session_id.has_value()
        ? Data::Models::Sessions::GetById(m_db, req.session_id.value())
        : Data::Models::Sessions::GetDefault(m_db);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    const auto& session_state = m_sessions.Get(session->id);

    if (session_state == nullptr)
    {
        return cb->Error(-2, "Session not loaded");
    }

    const auto torrent = session_state->Find(req.info_hash);

    if (torrent == nullptr)
    {
        return cb->Error(-3, "Torrent not found in session");
    }

    if (!torrent->status.handle.is_valid())
    {
        return cb->Error(-4, "Torrent not valid");
    }

    // check all limits
    const auto download_limit  = req.download_limit  ? Utils::CheckRateLimit(*req.download_limit)  : std::nullopt;
    const auto upload_limit    = req.upload_limit    ? Utils::CheckRateLimit(*req.upload_limit)    : std::nullopt;
    const auto max_connections = req.max_connections ? Utils::CheckPeerLimit(*req.max_connections) : std::nullopt;
    const auto max_uploads     = req.max_uploads     ? Utils::CheckPeerLimit(*req.max_uploads)     : std::nullopt;

    if (req.download_limit  && !download_limit)  return cb->Error(-5, "Invalid download_limit");
    if (req.upload_limit    && !upload_limit)    return cb->Error(-5, "Invalid upload_limit");
    if (req.max_connections && !max_connections) return cb->Error(-5, "Invalid max_connections");
    if (req.max_uploads     && !max_uploads)     return cb->Error(-5, "Invalid max_uploads");

    if (req.flags.has_value() && req.flags_mask.has_value())
    {
        const auto flags = req.flags.value();
        const auto mask  = req.flags_mask.value();

        torrent->status.handle.set_flags(flags, mask);
    }

    if (download_limit)  torrent->status.handle.set_download_limit(*download_limit);
    if (max_connections) torrent->status.handle.set_max_connections(*max_connections);
    if (max_uploads)     torrent->status.handle.set_max_uploads(*max_uploads);
    if (upload_limit)    torrent->status.handle.set_upload_limit(*upload_limit);

    if (req.category.has_value() || req.tags.has_value())
    {
        const bool updated = m_torrents.UpdateClientData(
            torrent->status.handle,
            [&req](TorrentClientData& client_data)
            {
                if (req.category.has_value()) client_data.category = req.category.value();
                if (req.tags.has_value())     client_data.tags     = req.tags.value();
            });

        if (!updated)
        {
            return cb->Error(-6, "Torrent has no client data - cannot set category or tags");
        }
    }

    cb->Ok({});
}
