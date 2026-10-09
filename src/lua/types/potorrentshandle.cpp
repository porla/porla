#include "potorrentshandle.hpp"

#include "ltaddtorrentparams.hpp"
#include "poquery.hpp"
#include "potorrentsiterator.hpp"

#include "../pluginstate.hpp"

#include "../../sessions/session.hpp"
#include "../../sessions/torrent.hpp"
#include "../../torrents.hpp"
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

std::tuple<sol::object, sol::object> PoTorrentsHandle::Add(sol::this_state ts, const sol::table& params, std::optional<sol::table> opts)
{
    sol::state_view lua(ts);

    auto session   = m_weak_session.lock();
    auto lua_state = lua.registry()["state"].get<std::weak_ptr<porla::Lua::LuaState>>().lock();

    if (session == nullptr || lua_state == nullptr)
    {
        return { sol::lua_nil, sol::make_object(lua, "Session not loaded") };
    }

    TorrentsAddOptions options{ .session_id = session->Id() };

    if (opts.has_value())
    {
        if (sol::optional<int> v = (*opts)["preset_id"])        options.preset_id = *v;
        else if (sol::optional<std::string> v = (*opts)["preset"]) options.preset = *v;
    }

    TorrentsAddResult      prepared;
    TorrentClientData*     prepared_data = nullptr;
    lt::add_torrent_params atp;

    try
    {
        atp = LtAddTorrentParams::ToParams(
            params,
            [&](lt::add_torrent_params& p)
            {
                prepared      = lua_state->torrents.Prepare(p, options);
                prepared_data = p.userdata.get<TorrentClientData>();
            });
    }
    catch (...)
    {
        // invalid params - free what Prepare allocated before rethrowing to Lua
        if (prepared) delete prepared_data;
        throw;
    }

    if (!prepared)
    {
        // Prepare failed and allocated nothing, but ToParams may have created client data
        delete atp.userdata.get<TorrentClientData>();
        return { sol::lua_nil, sol::make_object(lua, prepared.what) };
    }

    const auto added = lua_state->torrents.Add(std::move(atp));

    if (!added)
    {
        return { sol::lua_nil, sol::make_object(lua, added.what) };
    }

    return { sol::make_object(lua, added.info_hash), sol::lua_nil };
}

int PoTorrentsHandle::Count()
{
    auto state = m_weak_session.lock();
    if (state == nullptr) { return -1; }

    return state->Count();
}

std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> PoTorrentsHandle::Get(const lt::info_hash_t& info_hash)
{
    auto state = m_weak_session.lock();
    if (state == nullptr) { return std::nullopt; }

    auto found = state->Find(info_hash);

    if (found == nullptr)
    {
        return std::nullopt;
    }

    return std::make_tuple(found->status.handle, found->status);
}

std::shared_ptr<PoTorrentsIterator> PoTorrentsHandle::List()
{
    auto state = m_weak_session.lock();
    if (state == nullptr) { return nullptr; }

    return std::make_shared<PoTorrentsIterator>(state, std::nullopt);
}

std::shared_ptr<PoTorrentsIterator> PoTorrentsHandle::List(const PoQuery& query)
{
    auto state = m_weak_session.lock();
    if (state == nullptr) { return nullptr; }

    return std::make_shared<PoTorrentsIterator>(state, query);
}

void PoTorrentsHandle::Remove(const lt::info_hash_t& ih, std::optional<sol::table> opts)
{
    auto state = m_weak_session.lock();
    if (state == nullptr) { return; }

    auto found = state->Find(ih);

    if (found == nullptr)
    {
        return;
    }

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

    state->Libtorrent().remove_torrent(
        found->status.handle,
        flags);
}

void PoTorrentsHandle::Remove(const lt::torrent_handle& th, std::optional<sol::table> opts)
{
    auto state = m_weak_session.lock();
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

    state->Libtorrent().remove_torrent(th, flags);
}
