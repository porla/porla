#include "ltsettings.hpp"

#include <libtorrent/alert_types.hpp>

#include "../buildinfo.hpp"

using porla::Utils::LibtorrentSettingsPack;

void LibtorrentSettingsPack::Update(lt::settings_pack& settings, const std::map<std::string, nlohmann::json>& values)
{
    for (const auto& [ key, value ] : values)
    {
        const int type = lt::setting_by_name(key.data());
        if (type == -1) { continue; }

        if ((type & lt::settings_pack::type_mask) == lt::settings_pack::bool_type_base)
        {
            if (!value.is_boolean()) { continue; }
            settings.set_bool(type, value.get<bool>());
        }
        else if ((type & lt::settings_pack::type_mask) == lt::settings_pack::int_type_base)
        {
            if (!value.is_number()) { continue; }
            settings.set_int(type, value.get<int>());
        }
        else if ((type & lt::settings_pack::type_mask) == lt::settings_pack::string_type_base)
        {
            if (!value.is_string()) { continue; }
            settings.set_str(type, value.get<std::string>());
        }
    }
}

void LibtorrentSettingsPack::UpdateStatic(lt::settings_pack& settings)
{
    lt::alert_category_t alerts =
        lt::alert::status_notification
        | lt::alert::storage_notification
        | lt::alert::tracker_notification;

    std::stringstream fingerprint;
    fingerprint << lt::generate_fingerprint(
        "PO",
        BuildInfo::VersionMajor(),
        BuildInfo::VersionMinor(),
        BuildInfo::VersionPatch());

    std::stringstream user_agent;
    user_agent << "porla/" << BuildInfo::Version() << " libtorrent/" << LIBTORRENT_VERSION;

    settings.set_int(lt::settings_pack::alert_mask,       alerts);
    settings.set_str(lt::settings_pack::peer_fingerprint, fingerprint.str());
    settings.set_str(lt::settings_pack::user_agent,       user_agent.str());
}
