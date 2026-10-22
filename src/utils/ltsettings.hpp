#pragma once

#include <map>
#include <string>

#include <libtorrent/settings_pack.hpp>
#include <nlohmann/json.hpp>

namespace porla::Utils
{
    class LibtorrentSettingsPack
    {
    public:
        static void Update(lt::settings_pack& settings, const std::map<std::string, nlohmann::json>& values);
        static void UpdateStatic(lt::settings_pack& settings);

        // The name of the first setting in the pack with a value libtorrent can't handle, if any.
        static std::optional<std::string> Validate(const lt::settings_pack& settings);

        // Resets settings Validate would reject to their libtorrent default, logging each one.
        static void Sanitize(lt::settings_pack& settings);
    };
}
