#include "potorrentdata.hpp"

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
        "tags", [](const TorrentClientData& d) { return sol::as_table(d.tags); }
    );
}
