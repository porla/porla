#include "torrentsresume.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsResume;
using porla::Methods::TorrentsResumeReq;
using porla::Methods::TorrentsResumeRes;

TorrentsResume::TorrentsResume(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsResume::Invoke(const TorrentsResumeReq& req, WriteCb<TorrentsResumeRes> cb)
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

    if (!th.is_valid())
    {
        return cb.Error(-2, "Torrent not valid");
    }

    th.resume();

    cb.Ok(TorrentsResumeRes{});
}
