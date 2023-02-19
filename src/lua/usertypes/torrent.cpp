#include "torrent.hpp"

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../../torrentclientdata.hpp"

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

    type["category"]  = sol::property(&Torrent::Category);
    type["name"]      = sol::property(&Torrent::Name);
    type["save_path"] = sol::property(&Torrent::SavePath);
    type["size"]      = sol::property(&Torrent::Size);
    type["tags"]      = sol::property(&Torrent::Tags);
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

std::vector<std::string> Torrent::Tags()
{
    const TorrentClientData* client_data = m_state->th.userdata().get<TorrentClientData>();
    const auto tags = client_data->tags.value_or(std::unordered_set<std::string>());
    return {tags.begin(), tags.end()};
}
