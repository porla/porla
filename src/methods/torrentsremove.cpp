#include "torrentsremove.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsRemove;
using porla::Methods::TorrentsRemoveReq;
using porla::Methods::TorrentsRemoveRes;

TorrentsRemove::TorrentsRemove(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsRemove::Invoke(const TorrentsRemoveReq &req, WriteCb<TorrentsRemoveRes> cb)
{
    auto session = m_sessions.Default();

    for (auto const& hash : req.info_hashes)
    {
        if (session->torrents.find(hash) == session->torrents.end())
        {
            continue;
        }

        session->session->remove_torrent(
            session->torrents.at(hash),
            req.remove_data ? lt::session::delete_files : lt::remove_flags_t{});
    }

    cb(TorrentsRemoveRes{});
}
