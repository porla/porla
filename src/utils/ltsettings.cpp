#include "ltsettings.hpp"

#include <libtorrent/alert_types.hpp>

#include "../buildinfo.hpp"

using porla::Utils::LibtorrentSettingsPack;

void LibtorrentSettingsPack::Update(lt::settings_pack& settings, const std::map<std::string, nlohmann::json>& values)
{
    
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
