#include "torrentsmetadatalist.hpp"

#include "../session.hpp"
#include "../torrentclientdata.hpp"

using porla::Methods::TorrentsMetadataList;
using porla::Methods::TorrentsMetadataListReq;
using porla::Methods::TorrentsMetadataListRes;

TorrentsMetadataList::TorrentsMetadataList(sqlite3 *db, ISession &session)
    : m_db(db)
    , m_session(session)
{
}

void TorrentsMetadataList::Invoke(const TorrentsMetadataListReq& req, WriteCb<TorrentsMetadataListRes> cb)
{
    const auto& torrents = m_session.Torrents();
    const auto handle = torrents.find(req.info_hash);

    if (handle == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    const auto client_data = handle->second.userdata().get<TorrentClientData>();

    return cb.Ok(TorrentsMetadataListRes{
        .metadata = client_data->metadata.has_value()
            ? client_data->metadata.value()
            : std::map<std::string, nlohmann::json>()
    });
}
