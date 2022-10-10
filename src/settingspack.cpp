#include "settingspack.hpp"

#include <libtorrent/session.hpp>

namespace lt = libtorrent;
using porla::SettingsPack;

lt::settings_pack SettingsPack::Load(const toml::table &cfg)
{
    const std::string base = cfg["libtorrent"]["base"].value_or("default");

    auto pack = base == "min_memory_usage"
        ? lt::min_memory_usage()
        : base == "high_performance_seed"
            ? lt::high_performance_seed()
            : lt::default_settings();

    return pack;
}
