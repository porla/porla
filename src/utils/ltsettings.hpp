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
    };
}
