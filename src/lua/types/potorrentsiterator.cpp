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

    auto next = m_last_hash.has_value()
        ? session->Torrents().upper_bound(m_last_hash.value())
        : session->Torrents().begin();

    while (next != session->Torrents().end())
    {
        const auto& ts = next->second.status;

        m_last_hash = next->first;

        if (!m_query.has_value() || m_query->Includes(ts))
        {
            return std::make_tuple(ts.handle, ts);
        }

        next++;
    }

    return std::nullopt;
}
