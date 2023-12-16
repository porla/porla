#include "torrentsmetadatalist.hpp"

#include "../sessions.hpp"
#include "../torrentclientdata.hpp"

using porla::Methods::TorrentsMetadataList;
using porla::Methods::TorrentsMetadataListReq;
using porla::Methods::TorrentsMetadataListRes;

TorrentsMetadataList::TorrentsMetadataList(sqlite3 *db, Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsMetadataList::Invoke(const TorrentsMetadataListReq& req, WriteCb<TorrentsMetadataListRes> cb)
{
    const auto& state = std::find_if(
        m_sessions.All().begin(),
        m_sessions.All().end(),
        [hash = req.info_hash](const auto& state)
        {
            return state.second->torrents.find(hash) != state.second->torrents.end();
        });

    if (state == m_sessions.All().end())
    {
        return cb.Error(-1, "Torrent not found in any session");
    }

    const auto& handle = state->second->torrents.find(req.info_hash);

    if (handle == state->second->torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    const auto& [ th, _ ] = handle->second;
    const auto& client_data = th.userdata().get<TorrentClientData>();

    return cb.Ok(TorrentsMetadataListRes{
        .metadata = client_data->metadata.has_value()
            ? client_data->metadata.value()
            : std::map<std::string, nlohmann::json>()
    });
}
