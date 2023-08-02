#include "torrentspropertiesget.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsPropertiesGet;
using porla::Methods::TorrentsPropertiesGetReq;
using porla::Methods::TorrentsPropertiesGetRes;

TorrentsPropertiesGet::TorrentsPropertiesGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPropertiesGet::Invoke(const TorrentsPropertiesGetReq& req, WriteCb<TorrentsPropertiesGetRes> cb)
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

    cb.Ok(TorrentsPropertiesGetRes{
        .download_limit  = handle->second.download_limit(),
        .flags           = handle->second.flags(),
        .max_connections = handle->second.max_connections(),
        .max_uploads     = handle->second.max_uploads(),
        .upload_limit    = handle->second.upload_limit()
    });
}
