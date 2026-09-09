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

std::optional<lt::torrent_handle> PoTorrentsHandle::Get(const std::string& info_hash)
{
    lt::sha1_hash hash;

    {
        std::stringstream ss(info_hash);
        ss >> hash;
    }

    auto state = m_state.lock();
    auto found = state->torrents.find(lt::info_hash_t(hash));

    if (found == state->torrents.end())
    {
        return std::nullopt;
    }

    auto [ th, _ ] = found->second;

    return th;
}

std::shared_ptr<PoTorrentsIterator> PoTorrentsHandle::List()
{
    return std::make_shared<PoTorrentsIterator>(m_state.lock()->torrents);
}