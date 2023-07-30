#include "torrentspropertiesset.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsPropertiesSet;
using porla::Methods::TorrentsPropertiesSetReq;
using porla::Methods::TorrentsPropertiesSetRes;

TorrentsPropertiesSet::TorrentsPropertiesSet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPropertiesSet::Invoke(const TorrentsPropertiesSetReq& req, WriteCb<TorrentsPropertiesSetRes> cb)
{
    auto& torrents = m_sessions.Default()->torrents;
    auto torrent = torrents.find(req.info_hash);

    if (torrent == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    auto& handle = torrent->second;

    if (auto val = req.download_limit)
        handle.set_download_limit(*val);

    if (auto val = req.set_flags)
        handle.set_flags(*val);

    if (auto val = req.max_connections)
        handle.set_max_connections(*val);

    if (auto val = req.max_uploads)
        handle.set_max_uploads(*val);

    if (auto val = req.upload_limit)
        handle.set_upload_limit(*val);

    if (auto val = req.unset_flags)
        handle.unset_flags(*val);

    cb.Ok({});
}
