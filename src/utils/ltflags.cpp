#include "ltflags.hpp"

using porla::Utils::LtFlags;

namespace
{
    const std::map<std::string, lt::torrent_flags_t> FlagsLookup =
    {
        { "seed_mode",             lt::torrent_flags::seed_mode },
        { "upload_mode",           lt::torrent_flags::upload_mode },
        { "share_mode",            lt::torrent_flags::share_mode },
        { "apply_ip_filter",       lt::torrent_flags::apply_ip_filter },
        { "paused",                lt::torrent_flags::paused },
        { "auto_managed",          lt::torrent_flags::auto_managed },
        { "duplicate_is_error",    lt::torrent_flags::duplicate_is_error },
        { "update_subscribe",      lt::torrent_flags::update_subscribe },
        { "super_seeding",         lt::torrent_flags::super_seeding },
        { "sequential_download",   lt::torrent_flags::sequential_download },
        { "stop_when_ready",       lt::torrent_flags::stop_when_ready },
        { "need_save_resume",      lt::torrent_flags::need_save_resume },
        { "disable_dht",           lt::torrent_flags::disable_dht },
        { "disable_lsd",           lt::torrent_flags::disable_lsd },
        { "disable_pex",           lt::torrent_flags::disable_pex },
        { "no_verify_files",       lt::torrent_flags::no_verify_files },
        { "default_dont_download", lt::torrent_flags::default_dont_download },
        { "i2p_torrent",           lt::torrent_flags::i2p_torrent },
        { "disable_v1_hashes",     lt::torrent_flags::disable_v1_hashes }
    };
}

const std::map<std::string, lt::torrent_flags_t>& LtFlags::AllTorrentFlags()
{
    return FlagsLookup;
}

std::optional<lt::torrent_flags_t> LtFlags::FindTorrentFlag(const std::string& name)
{
    const auto it = FlagsLookup.find(name);

    if (it == FlagsLookup.end())
    {
        return std::nullopt;
    }

    return it->second;
}
