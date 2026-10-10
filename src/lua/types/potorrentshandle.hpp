#pragma once

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>
#include <sol/sol.hpp>

namespace porla
{
    class Session;
}

namespace porla::Lua::Types
{
    class PoQuery;
    class PoTorrentsIterator;

    class PoTorrentsHandle
    {
    public:
        static void Register(sol::state& lua);

        explicit PoTorrentsHandle(std::weak_ptr<Session> weak_session)
            : m_weak_session(weak_session) {}

        std::tuple<sol::object, sol::object> Add(sol::this_state ts, const sol::table& params, std::optional<sol::table> opts);

        int Count();

        std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> Get(const lt::info_hash_t& info_hash);

        std::shared_ptr<PoTorrentsIterator> List();
        std::shared_ptr<PoTorrentsIterator> List(const PoQuery& query);

        void Remove(const lt::info_hash_t& ih, std::optional<sol::table> opts);
        void Remove(const lt::torrent_handle& th, std::optional<sol::table> opts);

    private:
        std::weak_ptr<Session> m_weak_session;
    };
}
