#include "torrentspropertiesset.hpp"

#include "../../sessions.hpp"

using porla::Methods::TorrentsPropertiesSet;
using porla::Methods::TorrentsPropertiesSetReq;
using porla::Methods::TorrentsPropertiesSetRes;

TorrentsPropertiesSet::TorrentsPropertiesSet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPropertiesSet::Invoke(const TorrentsPropertiesSetReq& req, WriteCb<TorrentsPropertiesSetRes> cb)
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

    if (const auto val = req.download_limit)
        th.set_download_limit(*val);

    if (const auto val = req.flags)
    {
        lt::torrent_flags_t flags = {};
        lt::torrent_flags_t mask  = {};

#define SET_FLAG(name) \
    if (flag == #name) { mask |= lt::torrent_flags:: name; if (set) { flags |= lt::torrent_flags:: name; } }

        for (const auto& [ flag, set ] : *val)
        {
            SET_FLAG(seed_mode)
            SET_FLAG(upload_mode)
            SET_FLAG(share_mode)
            SET_FLAG(apply_ip_filter)
            SET_FLAG(paused)
            SET_FLAG(auto_managed)
            SET_FLAG(duplicate_is_error)
            SET_FLAG(update_subscribe)
            SET_FLAG(super_seeding)
            SET_FLAG(sequential_download)
            SET_FLAG(stop_when_ready)
            SET_FLAG(override_trackers)
            SET_FLAG(override_web_seeds)
            SET_FLAG(need_save_resume)
            SET_FLAG(disable_dht)
            SET_FLAG(disable_lsd)
            SET_FLAG(disable_pex)
            SET_FLAG(no_verify_files)
            SET_FLAG(default_dont_download)
            SET_FLAG(i2p_torrent)
        }

        th.set_flags(flags, mask);
    }

    if (const auto val = req.max_connections)
        th.set_max_connections(*val);

    if (const auto val = req.max_uploads)
        th.set_max_uploads(*val);

    if (const auto val = req.upload_limit)
        th.set_upload_limit(*val);

    cb.Ok({});
}
