#include "inmemorysession.hpp"

libtorrent::info_hash_t InMemorySession::AddTorrent(const libtorrent::add_torrent_params &p)
{
    return {};
}

void InMemorySession::ApplySettings(const libtorrent::settings_pack &settings)
{
}

void InMemorySession::Pause()
{
}

void InMemorySession::Recheck(const lt::info_hash_t &hash)
{
}

void InMemorySession::Remove(const lt::info_hash_t &hash, bool remove_data)
{
}

void InMemorySession::Resume()
{
}

libtorrent::settings_pack InMemorySession::Settings()
{
    return {};
}

const std::map<lt::info_hash_t, lt::torrent_handle> &InMemorySession::Torrents()
{
    return m_torrents;
}
