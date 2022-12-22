#include "torrentsmetadatalist.hpp"

#include "../data/models/torrentsmetadata.hpp"
#include "../session.hpp"

using porla::Data::Models::TorrentsMetadata;
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
    auto const& torrents = m_session.Torrents();
    auto const handle = torrents.find(req.info_hash);

    if (handle == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    return cb.Ok(TorrentsMetadataListRes{
        .metadata = TorrentsMetadata::GetAll(m_db, req.info_hash)
    });
}
