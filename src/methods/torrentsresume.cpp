#include "torrentsresume.hpp"

#include "../session.hpp"

using porla::Methods::TorrentsResume;
using porla::Methods::TorrentsResumeReq;
using porla::Methods::TorrentsResumeRes;

TorrentsResume::TorrentsResume(porla::ISession& session)
    : m_session(session)
{
}

void TorrentsResume::Invoke(const TorrentsResumeReq& req, WriteCb<TorrentsResumeRes> cb)
{
    auto const& torrents = m_session.Torrents();
    auto status = torrents.find(req.info_hash);

    if (status == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    status->second.resume();

    cb.Ok(TorrentsResumeRes{});
}
