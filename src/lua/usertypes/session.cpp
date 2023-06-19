#include "session.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/torrent_info.hpp>

#include "../../session.hpp"
#include "../../torrentclientdata.hpp"

using porla::Lua::UserTypes::Session;

void Session::Register(sol::state& lua)
{
    sol::usertype<Session> type = lua.new_usertype<Session>(
        "porla.Session",
        sol::no_constructor);

    type["addTorrent"] = [](const Session& self, const sol::table& args)
    {
        lt::add_torrent_params p;
        p.userdata = lt::client_data_t(new TorrentClientData());
        p.save_path = args["path"];
        p.ti = args["torrent"];

        self.m_session.AddTorrent(p);
    };

    type["hasTorrent"] = [](const Session& self, const std::shared_ptr<lt::torrent_info>& ti)
    {
        return self.m_session.Torrents().find(ti->info_hashes()) != self.m_session.Torrents().end();
    };
}

Session::Session(ISession& session)
    : m_session(session)
{}
