#include "../all.hpp"

#include <unordered_set>

#include <libtorrent/torrent_flags.hpp>

static const std::unordered_map<std::string, lt::torrent_flags_t> FlagsLookup =
{
    { "seed_mode",           lt::torrent_flags::seed_mode },
    { "upload_mode",         lt::torrent_flags::upload_mode },
    { "share_mode",          lt::torrent_flags::share_mode },
    { "apply_ip_filter",     lt::torrent_flags::apply_ip_filter },
    { "paused",              lt::torrent_flags::paused },
    { "auto_managed",        lt::torrent_flags::auto_managed },
    { "duplicate_is_error",  lt::torrent_flags::duplicate_is_error },
    { "update_subscribe",    lt::torrent_flags::update_subscribe },
    { "super_seeding",       lt::torrent_flags::super_seeding },
    { "sequential_download", lt::torrent_flags::sequential_download },
    { "stop_when_ready",     lt::torrent_flags::stop_when_ready },
    { "need_save_resume",    lt::torrent_flags::need_save_resume },
    { "disable_dht",         lt::torrent_flags::disable_dht },
    { "disable_lsd",         lt::torrent_flags::disable_lsd },
    { "disable_pex",         lt::torrent_flags::disable_pex },
    { "no_verify_files",     lt::torrent_flags::no_verify_files },
    { "i2p_torrent",         lt::torrent_flags::i2p_torrent }
};

namespace libtorrent
{
    void from_json(const nlohmann::json& j, torrent_flags_t& flags)
    {
        flags = {};

        if (j.is_array())
        {
            // the input should be an array of strings
            // [ "seed_mode", "upload_mode" ]

            const auto input_flags = j.get<std::unordered_set<std::string>>();

            for (const auto& ifl : input_flags)
            {
                flags |= FlagsLookup.at(ifl);
            }
        }
        else if (j.is_object())
        {
            // the input should be an object of string/bool pairs
            // { "seed_mode": true, "upload_mode": false }
        }
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
