#include "torrentspropertiesget.hpp"

#include "../../sessions.hpp"

using porla::Methods::TorrentsPropertiesGet;
using porla::Methods::TorrentsPropertiesGetReq;
using porla::Methods::TorrentsPropertiesGetRes;

TorrentsPropertiesGet::TorrentsPropertiesGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPropertiesGet::Invoke(const TorrentsPropertiesGetReq& req, WriteCb<TorrentsPropertiesGetRes> cb)
{
    const auto& session_state = req.session_id.has_value()
        ? m_sessions.Get(req.session_id.value())
        : m_sessions.Default();

    if (session_state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    const auto& handle = session_state->torrents.find(req.info_hash);

    if (handle == session_state->torrents.end())
    {
        return cb.Error(-2, "Torrent not found in session");
    }

    const auto& [ th, _ ] = handle->second;

    if (!th.is_valid())
    {
        return cb.Error(-3, "Torrent not valid");
    }

    const auto handle_flags = th.flags();

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
    INSERT_FLAG(override_trackers)
    INSERT_FLAG(override_web_seeds)
    INSERT_FLAG(need_save_resume)
    INSERT_FLAG(disable_dht)
    INSERT_FLAG(disable_lsd)
    INSERT_FLAG(disable_pex)
    INSERT_FLAG(no_verify_files)
    INSERT_FLAG(default_dont_download)
    INSERT_FLAG(i2p_torrent)

    cb.Ok(TorrentsPropertiesGetRes{
        .download_limit  = th.download_limit(),
        .flags           = flags,
        .max_connections = th.max_connections(),
        .max_uploads     = th.max_uploads(),
        .upload_limit    = th.upload_limit()
    });
}
