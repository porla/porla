#include "torrentspropertiesset.hpp"

#include "resolve.hpp"

#include "../../../data/models/addtorrentparams.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"
#include "../../../torrents.hpp"
#include "../../../torrentclientdata.hpp"
#include "../../../utils/limits.hpp"

using porla::Rpc::Methods::Torrents::TorrentsPropertiesSet;
using porla::Rpc::Methods::Torrents::TorrentsPropertiesSetReq;
using porla::Rpc::Methods::Torrents::TorrentsPropertiesSetRes;

TorrentsPropertiesSet::TorrentsPropertiesSet(porla::Sessions& sessions, porla::Torrents& torrents)
    : m_sessions(sessions)
    , m_torrents(torrents)
{
}

void TorrentsPropertiesSet::Execute(const TorrentsPropertiesSetReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
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

        resolved->torrent->status.handle.set_flags(flags, mask);
    }

    if (download_limit)  resolved->torrent->status.handle.set_download_limit(*download_limit);
    if (max_connections) resolved->torrent->status.handle.set_max_connections(*max_connections);
    if (max_uploads)     resolved->torrent->status.handle.set_max_uploads(*max_uploads);
    if (upload_limit)    resolved->torrent->status.handle.set_upload_limit(*upload_limit);

    if (req.category.has_value() || req.tags.has_value())
    {
        const bool updated = m_torrents.UpdateClientData(
            resolved->torrent->status.handle,
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
