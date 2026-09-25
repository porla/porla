#include "potorrentdata.hpp"

#include "pojson.hpp"
#include "posessionhandle.hpp"
#include "../../torrentclientdata.hpp"

using porla::Lua::Types::PoTorrentData;
using porla::TorrentClientData;

void PoTorrentData::Register(sol::state& lua)
{
    lua.new_usertype<PoTorrentData>(
        "PoTorrentData",
        sol::no_constructor,
        "add_tag", [](const PoTorrentData& d, const std::string& tag)
        {
            d.ClientData().tags.insert(tag);
        },
        "category", sol::property(
            [](const PoTorrentData& d) { return d.ClientData().category; },
            [](const PoTorrentData& d, std::optional<std::string> v) { d.ClientData().category = std::move(v); }),
        "has_tag", [](const PoTorrentData& d, const std::string& tag)
        {
            return d.ClientData().tags.contains(tag);
        },
        "remove_tag", [](const PoTorrentData& d, const std::string& tag)
        {
            d.ClientData().tags.erase(tag);
        },
        "session", sol::property([](const PoTorrentData& ptd) { return std::make_shared<PoSessionHandle>(ptd.ClientData().state); }),
        "get_metadata", [](const PoTorrentData& d, const std::string& key, sol::this_state ts) -> std::optional<sol::object>
        {
            const auto& cd  = d.ClientData();
            const auto  val = cd.metadata.find(key);

            if (val != cd.metadata.end())
            {
                if (val->second.is_null())
                {
                    return sol::lua_nil;
                }

                return PoJson::ToLua(ts, val->second, 0);
            }

            return std::nullopt;
        },
        "set_metadata", [](const PoTorrentData& d, const std::string& key, sol::object value, sol::this_state ts)
        {
            d.ClientData().metadata[key] = PoJson::ToJson(ts, value, 0);
        },
        "tags", [](const PoTorrentData& d) { return sol::as_table(d.ClientData().tags); }
    );
}

PoTorrentData::PoTorrentData(lt::torrent_handle th)
    : m_th(th)
{
}

TorrentClientData& PoTorrentData::ClientData() const
{
    auto* data = m_th.userdata().get<TorrentClientData>();

    if (data == nullptr)
    {
        throw sol::error("Torrent is no longer in the session");
    }

    return *data;
}
