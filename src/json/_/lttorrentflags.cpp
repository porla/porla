#include "../all.hpp"

#include <libtorrent/torrent_flags.hpp>

namespace libtorrent
{
    void from_json(const nlohmann::json& j, torrent_flags_t& flags)
    {

    }

    void to_json(nlohmann::json& j, const torrent_flags_t& flags)
    {
#define JSON_FLAG(name) { #name, (flags & lt::torrent_flags:: name) == lt::torrent_flags:: name },

        j = {
            JSON_FLAG(seed_mode)
            JSON_FLAG(upload_mode)
            JSON_FLAG(share_mode)
            JSON_FLAG(apply_ip_filter)
            JSON_FLAG(paused)
            JSON_FLAG(auto_managed)
            JSON_FLAG(duplicate_is_error)
            JSON_FLAG(update_subscribe)
            JSON_FLAG(super_seeding)
            JSON_FLAG(sequential_download)
            JSON_FLAG(stop_when_ready)
            JSON_FLAG(override_trackers)
            JSON_FLAG(override_web_seeds)
            JSON_FLAG(need_save_resume)
            JSON_FLAG(disable_dht)
            JSON_FLAG(disable_lsd)
            JSON_FLAG(disable_pex)
            JSON_FLAG(no_verify_files)
            JSON_FLAG(default_dont_download)
            JSON_FLAG(i2p_torrent)
        };
    }
}
