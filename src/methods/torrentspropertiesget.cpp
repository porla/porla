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

    const auto handle_flags = handle->second.flags();

#define INSERT_FLAG(name) flags.insert({ #name, (handle_flags & lt::torrent_flags:: name) == lt::torrent_flags:: name });

    std::map<std::string, bool> flags;
    INSERT_FLAG(seed_mode)
    INSERT_FLAG(upload_mode)
    INSERT_FLAG(share_mode)
    INSERT_FLAG(apply_ip_filter)
    INSERT_FLAG(paused)
    INSERT_FLAG(auto_managed)
    INSERT_FLAG(duplicate_is_error)
    INSERT_FLAG(update_subscribe)
    INSERT_FLAG(super_seeding)
    INSERT_FLAG(sequential_download)
    INSERT_FLAG(stop_when_ready)
    INSERT_FLAG(need_save_resume)
    INSERT_FLAG(disable_dht)
    INSERT_FLAG(disable_lsd)
    INSERT_FLAG(disable_pex)
    INSERT_FLAG(no_verify_files)
    INSERT_FLAG(default_dont_download)
    INSERT_FLAG(i2p_torrent)

    cb.Ok(TorrentsPropertiesGetRes{
        .download_limit  = handle->second.download_limit(),
        .flags           = flags,
        .max_connections = handle->second.max_connections(),
        .max_uploads     = handle->second.max_uploads(),
        .upload_limit    = handle->second.upload_limit()
    });
}
