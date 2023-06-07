#include "torrent.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../../torrentclientdata.hpp"
#include "../../utils/ratio.hpp"

using porla::Lua::UserTypes::Torrent;
using porla::TorrentClientData;

struct Torrent::State
{
    explicit State(const lt::torrent_handle& th)
        : th(th)
        , ts(th.status())
    {
    }

    lt::torrent_handle th;
    lt::torrent_status ts;
};

void Torrent::Register(sol::state &lua)
{
    sol::usertype<Torrent> type = lua.new_usertype<Torrent>(
        "porla_torrent",
        sol::no_constructor);

    type["category"]       = sol::property(&Torrent::Category);
    type["is_downloading"] = sol::property([](const Torrent& self) { return self.m_state->ts.state == lt::torrent_status::downloading; });
    type["is_finished"]    = sol::property([](const Torrent& self) { return self.m_state->ts.state == lt::torrent_status::finished; });
    type["is_moving"]      = sol::property([](const Torrent& self) { return self.m_state->ts.moving_storage; });
    type["is_paused"]      = sol::property([](const Torrent& self) { return (self.m_state->ts.flags & lt::torrent_flags::paused) == lt::torrent_flags::paused; });
    type["is_seeding"]     = sol::property([](const Torrent& self) { return self.m_state->ts.state == lt::torrent_status::seeding; });
    type["name"]           = sol::property(&Torrent::Name);
    type["ratio"]          = sol::property([](const Torrent& self) { return porla::Utils::Ratio(self.m_state->ts); });
    type["save_path"]      = sol::property(&Torrent::SavePath);

    type["active_duration"] = sol::property([](const Torrent& self)
    {
        return self.m_state->ts.active_duration.count();
    });

    type["finished_duration"] = sol::property([](const Torrent& self)
    {
        return self.m_state->ts.finished_duration.count();
    });

    type["seeding_duration"] = sol::property([](const Torrent& self)
    {
        return self.m_state->ts.seeding_duration.count();
    });

    type["size"]       = sol::property(&Torrent::Size);
    type["tags"]       = sol::property(&Torrent::Tags, &Torrent::TagsSet);
}

Torrent::Torrent(const lt::torrent_handle &th)
    : m_state(std::make_shared<State>(th))
{
}

Torrent::~Torrent() = default;

std::optional<std::string> Torrent::Category()
{
    const TorrentClientData* client_data = m_state->th.userdata().get<TorrentClientData>();
    return client_data->category;
}

std::string Torrent::Name()
{
    return m_state->ts.name;
}

std::string Torrent::SavePath()
{
    return m_state->ts.save_path;
}

std::optional<std::int64_t> Torrent::Size()
{
    if (const auto tf = m_state->ts.torrent_file.lock())
    {
        return tf->total_size();
    }

    return std::nullopt;
}

std::unordered_set<std::string>& Torrent::Tags()
{
    auto client_data = m_state->th.userdata().get<TorrentClientData>();

    if (!client_data->tags.has_value())
    {
        client_data->tags = std::unordered_set<std::string>();
    }

    return *client_data->tags;
}

void Torrent::TagsSet(const std::unordered_set<std::string>& value)
{
    auto client_data = m_state->th.userdata().get<TorrentClientData>();
    client_data->tags = value;
}
