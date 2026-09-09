#include "potorrentsiterator.hpp"

using porla::Lua::Types::PoTorrentsIterator;

std::optional<lt::torrent_handle> PoTorrentsIterator::operator()()
{
    if (m_iterator == m_torrents.end())
    {
        return std::nullopt;
    }

    auto [ th, _ ] = m_iterator->second;
    std::advance(m_iterator, 1);

    return th;
}
