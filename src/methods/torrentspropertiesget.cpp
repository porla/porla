#include "torrentspropertiesget.hpp"

#include "../session.hpp"

using porla::Methods::TorrentsPropertiesGet;
using porla::Methods::TorrentsPropertiesGetReq;
using porla::Methods::TorrentsPropertiesGetRes;

TorrentsPropertiesGet::TorrentsPropertiesGet(porla::ISession& session)
    : m_session(session)
{
}

void TorrentsPropertiesGet::Invoke(const TorrentsPropertiesGetReq& req, WriteCb<TorrentsPropertiesGetRes> cb)
{
    auto const& torrents = m_session.Torrents();
    auto const& torrent = torrents.find(req.info_hash);
    auto const& handle = torrent->second;

    cb.Ok(TorrentsPropertiesGetRes{
        .download_limit  = handle.download_limit(),
        .flags           = handle.flags(),
        .max_connections = handle.max_connections(),
        .max_uploads     = handle.max_uploads(),
        .upload_limit    = handle.upload_limit()
    });
}
