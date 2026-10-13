#include "../types.hpp"

#include <libtorrent/torrent_flags.hpp>

#include "../../utils/ltflags.hpp"

using porla::Lua::Types::LtTorrentFlags;
using porla::Utils::LtFlags;

void LtTorrentFlags::Register(sol::state& lua)
{
    lua.new_usertype<lt::torrent_flags_t>(
        "LtTorrentFlags",
        sol::call_constructor, sol::factories([]() { return lt::torrent_flags_t{}; }),
        sol::meta_function::to_string, [](const lt::torrent_flags_t& flags)
        {
            std::string out;

            for (const auto& [name, flag] : LtFlags::AllTorrentFlags())
            {
                if ((flags & flag) == flag)
                {
                    if (!out.empty()) out += "|";
                    out += name;
                }
            }

            return out.empty() ? "(none)" : out;
        },
        "has", [](const lt::torrent_flags_t& flags, const std::string& flag)
        {
            const auto flag_value = LtFlags::FindTorrentFlag(flag);
            if (!flag_value.has_value()) { throw sol::error("invalid torrent flag: " + flag); }
            return (flags & *flag_value) == *flag_value;
        },
        "set", [](lt::torrent_flags_t& flags, const std::string& flag)
        {
            const auto flag_value = LtFlags::FindTorrentFlag(flag);
            if (!flag_value.has_value()) { throw sol::error("invalid torrent flag: " + flag); }
            flags |= *flag_value;
        },
        "unset", [](lt::torrent_flags_t& flags, const std::string& flag)
        {
            const auto flag_value = LtFlags::FindTorrentFlag(flag);
            if (!flag_value.has_value()) { throw sol::error("invalid torrent flag: " + flag); }
            flags &= ~*flag_value;
        }
    );
}
