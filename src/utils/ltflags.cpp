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

std::string LtFlags::ConnectionTypeName(const lt::connection_type_t type)
{
    if (type == lt::peer_info::web_seed)  return "web_seed";
    if (type == lt::peer_info::http_seed) return "http_seed";

    return "standard_bittorrent";
}

std::vector<std::string> LtFlags::PeerFlagNames(const lt::peer_flags_t flags)
{
    static const std::pair<lt::peer_flags_t, const char*> kNames[] =
    {
        {lt::peer_info::interesting,         "interesting"},
        {lt::peer_info::choked,              "choked"},
        {lt::peer_info::remote_interested,   "remote_interested"},
        {lt::peer_info::remote_choked,       "remote_choked"},
        {lt::peer_info::supports_extensions, "supports_extensions"},
        {lt::peer_info::outgoing_connection, "outgoing_connection"},
        {lt::peer_info::handshake,           "handshake"},
        {lt::peer_info::connecting,          "connecting"},
        {lt::peer_info::on_parole,           "on_parole"},
        {lt::peer_info::seed,                "seed"},
        {lt::peer_info::optimistic_unchoke,  "optimistic_unchoke"},
        {lt::peer_info::snubbed,             "snubbed"},
        {lt::peer_info::upload_only,         "upload_only"},
        {lt::peer_info::endgame_mode,        "endgame_mode"},
        {lt::peer_info::holepunched,         "holepunched"},
        {lt::peer_info::i2p_socket,          "i2p_socket"},
        {lt::peer_info::utp_socket,          "utp_socket"},
        {lt::peer_info::ssl_socket,          "ssl_socket"},
        {lt::peer_info::rc4_encrypted,       "rc4_encrypted"},
        {lt::peer_info::plaintext_encrypted, "plaintext_encrypted"},
    };

    std::vector<std::string> names;

    for (const auto& [flag, name] : kNames)
    {
        if (flags & flag) names.emplace_back(name);
    }

    return names;
}

std::vector<std::string> LtFlags::PeerSourceNames(const lt::peer_source_flags_t source)
{
    static const std::pair<lt::peer_source_flags_t, const char*> kNames[] =
    {
        {lt::peer_info::tracker,     "tracker"},
        {lt::peer_info::dht,         "dht"},
        {lt::peer_info::pex,         "pex"},
        {lt::peer_info::lsd,         "lsd"},
        {lt::peer_info::resume_data, "resume_data"},
        {lt::peer_info::incoming,    "incoming"},
    };

    std::vector<std::string> names;

    for (const auto& [flag, name] : kNames)
    {
        if (source & flag) names.emplace_back(name);
    }

    return names;
}
