#include "potorrentdata.hpp"

#include "pojson.hpp"
#include "posessionhandle.hpp"
#include "../../torrentclientdata.hpp"

using porla::Lua::Types::PoTorrentData;

void PoTorrentData::Register(sol::state& lua)
{
    lua.new_usertype<TorrentClientData>(
        "PoTorrentData",
        sol::no_constructor,
        "add_tag", [](TorrentClientData& d, const std::string& tag)
        {
            d.tags.insert(tag);
        },
        "category", &TorrentClientData::category,
        "has_tag", [](const TorrentClientData& d, const std::string& tag)
        {
            return d.tags.contains(tag);
        },
        "remove_tag", [](TorrentClientData& d, const std::string& tag)
        {
            d.tags.erase(tag);
        },
        "session", sol::property([](const TorrentClientData& tcd) { return std::make_shared<PoSessionHandle>(tcd.state); }),
        "get_metadata", [](const TorrentClientData& d, const std::string& key, sol::this_state ts) -> std::optional<sol::object>
        {
            const auto val = d.metadata.find(key);

            if (val != d.metadata.end())
            {
                return PoJson::ToLua(ts, val->second, 0);
            }

            return std::nullopt;
        },
        "set_metadata", [](TorrentClientData& d, const std::string& key, sol::object value, sol::this_state ts)
        {
            d.metadata[key] = PoJson::ToJson(ts, value, 0);
        },
        "tags", [](const TorrentClientData& d) { return sol::as_table(d.tags); }
    );
}
