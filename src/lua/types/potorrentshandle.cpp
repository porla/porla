#include "potorrentshandle.hpp"

#include "ltaddtorrentparams.hpp"
#include "poquery.hpp"
#include "potorrentsiterator.hpp"

#include "../../torrentclientdata.hpp"

using porla::Lua::Types::PoTorrentsHandle;
using porla::Lua::Types::PoTorrentsIterator;

void PoTorrentsHandle::Register(sol::state& lua)
{
    lua.new_usertype<PoTorrentsHandle>(
        "PoTorrentsHandle",
        sol::no_constructor,
        "add",    &PoTorrentsHandle::Add,
        "count",  &PoTorrentsHandle::Count,
        "get",    &PoTorrentsHandle::Get,
        "list",   sol::overload(
            sol::resolve<std::shared_ptr<PoTorrentsIterator>()>(&PoTorrentsHandle::List),
            sol::resolve<std::shared_ptr<PoTorrentsIterator>(const PoQuery&)>(&PoTorrentsHandle::List)
        ),
        "remove", sol::overload(
            sol::resolve<void(const lt::info_hash_t&, std::optional<sol::table>)>(&PoTorrentsHandle::Remove),
            sol::resolve<void(const lt::torrent_handle&, std::optional<sol::table>)>(&PoTorrentsHandle::Remove))
        );
}

void PoTorrentsHandle::Add(const sol::table& params)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    lt::add_torrent_params atp = LtAddTorrentParams::ToParams(params);
    atp.userdata = lt::client_data_t(new TorrentClientData());
    atp.userdata.get<TorrentClientData>()->state = m_state;

    state->session->async_add_torrent(atp);
}

int PoTorrentsHandle::Count()
{
    auto state = m_state.lock();
    if (state == nullptr) { return -1; }

    return state->torrents.size();
}

std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> PoTorrentsHandle::Get(const lt::info_hash_t& info_hash)
{
    auto state = m_state.lock();
    if (state == nullptr) { return std::nullopt; }

    auto found = state->torrents.find(info_hash);

    if (found == state->torrents.end())
    {
        return std::nullopt;
    }

    return found->second;
}

std::shared_ptr<PoTorrentsIterator> PoTorrentsHandle::List()
{
    auto state = m_state.lock();
    if (state == nullptr) { return nullptr; }

    return std::make_shared<PoTorrentsIterator>(state->torrents, std::nullopt);
}

std::shared_ptr<PoTorrentsIterator> PoTorrentsHandle::List(const PoQuery& query)
{
    auto state = m_state.lock();
    if (state == nullptr) { return nullptr; }

    return std::make_shared<PoTorrentsIterator>(state->torrents, query);
}

void PoTorrentsHandle::Remove(const lt::info_hash_t& ih, std::optional<sol::table> opts)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    auto found = state->torrents.find(ih);

    if (found == state->torrents.end())
    {
        return;
    }

    const auto [ th, _ ] = found->second;

    lt::remove_flags_t flags = {};

    if (opts.has_value())
    {
        sol::object delete_files = (*opts)["delete_files"];

        if (delete_files.valid() && delete_files.get_type() != sol::type::lua_nil)
        {
            if (delete_files.get_type() != sol::type::boolean)
            {
                throw std::runtime_error("'delete_files' parameter must be boolean");
            }

            if (delete_files.as<bool>())
            {
                flags |= lt::session::delete_files;
            }
        }
    }

    state->session->remove_torrent(th, flags);
}

void PoTorrentsHandle::Remove(const lt::torrent_handle& th, std::optional<sol::table> opts)
{
    auto state = m_state.lock();
    if (state == nullptr) { return; }

    lt::remove_flags_t flags = {};

    if (opts.has_value())
    {
        sol::object delete_files = (*opts)["delete_files"];

        if (delete_files.valid() && delete_files.get_type() != sol::type::lua_nil)
        {
            if (delete_files.get_type() != sol::type::boolean)
            {
                throw std::runtime_error("'delete_files' parameter must be boolean");
            }

            if (delete_files.as<bool>())
            {
                flags |= lt::session::delete_files;
            }
        }
    }

    state->session->remove_torrent(th, flags);
}