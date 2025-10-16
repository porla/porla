#include "torrentsmetadatalist.hpp"

#include "../../sessions.hpp"
#include "../../torrentclientdata.hpp"

using porla::Methods::TorrentsMetadataList;
using porla::Methods::TorrentsMetadataListReq;
using porla::Methods::TorrentsMetadataListRes;

TorrentsMetadataList::TorrentsMetadataList(sqlite3 *db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsMetadataList::Invoke(const TorrentsMetadataListReq& req, WriteCb<TorrentsMetadataListRes> cb)
{
    const auto& session_state = req.session_id.has_value()
        ? m_sessions.Get(req.session_id.value())
        : m_sessions.Default();

    if (session_state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    const auto& handle = session_state->torrents.find(req.info_hash);

    if (handle == session_state->torrents.end())
    {
        return cb.Error(-2, "Torrent not found in session");
    }

    const auto& [ th, _ ] = handle->second;
    const auto& client_data = th.userdata().get<TorrentClientData>();

    return cb.Ok(TorrentsMetadataListRes{
        .metadata = client_data->metadata.has_value()
            ? client_data->metadata.value()
            : std::map<std::string, nlohmann::json>()
    });
}
