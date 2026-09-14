#include "potorrentsiterator.hpp"

using porla::Lua::Types::PoTorrentsIterator;

std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> PoTorrentsIterator::operator()()
{
    while (m_iterator != m_torrents.end())
    {
        const auto& [th, ts] = m_iterator->second;

        ++m_iterator;

        if (!m_query.has_value() || m_query->Includes(ts))
        {
            return std::make_tuple(th, ts);
        }
    }

    return std::nullopt;
}
