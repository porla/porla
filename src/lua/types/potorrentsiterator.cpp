#include "potorrentsiterator.hpp"

#include "../../sessions/session.hpp"
#include "../../sessions/torrent.hpp"

using porla::Lua::Types::PoTorrentsIterator;

std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> PoTorrentsIterator::operator()()
{
    const auto session = m_weak_session.lock();

    if (session == nullptr)
    {
        return std::nullopt;
    }

    auto next = [&](auto torrents) -> std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>>
    {
        for (const auto& [ hash, torrent ] : torrents)
        {
            m_last_hash = hash;

            if (!m_query.has_value() || m_query->Includes(torrent.status))
            {
                return std::make_tuple(torrent.status.handle, torrent.status);
            }
        }

        return std::nullopt;
    };

    return m_last_hash.has_value()
        ? next(session->TorrentsAfter(m_last_hash.value()))
        : next(session->Torrents());
}
