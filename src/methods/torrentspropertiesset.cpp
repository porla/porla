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

    if (const auto val = req.download_limit)
        handle->second.set_download_limit(*val);

    if (const auto val = req.set_flags)
        handle->second.set_flags(*val);

    if (const auto val = req.max_connections)
        handle->second.set_max_connections(*val);

    if (const auto val = req.max_uploads)
        handle->second.set_max_uploads(*val);

    if (const auto val = req.upload_limit)
        handle->second.set_upload_limit(*val);

    if (const auto val = req.unset_flags)
        handle->second.unset_flags(*val);

    cb.Ok({});
}
