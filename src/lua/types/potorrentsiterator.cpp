#include "potorrentsiterator.hpp"

using porla::Lua::Types::PoTorrentsIterator;

std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> PoTorrentsIterator::operator()()
{
    const auto state = m_state.lock();

    if (state == nullptr)
    {
        return std::nullopt;
    }

    auto next = m_last_hash.has_value()
        ? state->torrents.upper_bound(m_last_hash.value())
        : state->torrents.begin();

    while (next != state->torrents.end())
    {
        const auto& ts = next->second;

        m_last_hash = next->first;

        if (!m_query.has_value() || m_query->Includes(ts))
        {
            return std::make_tuple(ts.handle, ts);
        }

        next++;
    }

    return std::nullopt;
}
