#include "../packages.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/file_storage.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../plugin.hpp"
#include "../../config.hpp"
#include "../../sessions.hpp"
#include "../../torrentclientdata.hpp"
#include "../../utils/ratio.hpp"

using porla::Lua::Packages::Torrents;

void Torrents::Register(sol::state& lua)
{
    auto torrent_client_data_type = lua.new_usertype<TorrentClientData>(
        "porla.TorrentClientData",
        sol::no_constructor,
        "category", &TorrentClientData::category,
        "session",  sol::property([](const TorrentClientData& cd) -> std::optional<std::string>
                    {
                        if (auto s = cd.state.lock())
                        {
                            return s->name;
                        }

                        return std::nullopt;
                    }),
        "tags",     &TorrentClientData::tags);
}
