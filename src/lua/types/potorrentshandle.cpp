#include "potorrentshandle.hpp"

#include "potorrentsiterator.hpp"

using porla::Lua::Types::PoTorrentsHandle;
using porla::Lua::Types::PoTorrentsIterator;

void PoTorrentsHandle::Register(sol::state& lua)
{
    lua.new_usertype<PoTorrentsHandle>(
        "PoTorrentsHandle",
        sol::no_constructor,
        "count", &PoTorrentsHandle::Count,
        "get",   &PoTorrentsHandle::Get,
        "list",  &PoTorrentsHandle::List);
}

int PoTorrentsHandle::Count()
{
    return m_state.lock()->torrents.size();
}

std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> PoTorrentsHandle::Get(const lt::info_hash_t& info_hash)
{
    auto state = m_state.lock();
    auto found = state->torrents.find(info_hash);

    if (found == state->torrents.end())
    {
        return std::nullopt;
    }

    return found->second;
}

std::shared_ptr<PoTorrentsIterator> PoTorrentsHandle::List()
{
    return std::make_shared<PoTorrentsIterator>(m_state.lock()->torrents);
}