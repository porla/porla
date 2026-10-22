#include "ltsettings.hpp"

#include <limits>

#include <boost/log/trivial.hpp>
#include <libtorrent/alert_types.hpp>

#include "../buildinfo.hpp"

using porla::Utils::LibtorrentSettingsPack;

namespace
{
    struct IntRange
    {
        int name;
        int min;
        int max;
    };

    // libtorrent's own ranges (session_impl::validate_settings, only asserted there), plus the
    // values that crash it outright
    const IntRange kIntRanges[] = {
        { lt::settings_pack::out_enc_policy,               0, 2 },
        { lt::settings_pack::in_enc_policy,                0, 2 },
        { lt::settings_pack::allowed_enc_level,            1, 3 },
        { lt::settings_pack::mixed_mode_algorithm,         0, 1 },
        { lt::settings_pack::proxy_type,                   0, 5 },
        { lt::settings_pack::disk_io_read_mode,            0, 3 },
        { lt::settings_pack::disk_io_write_mode,           0, 3 },
        { lt::settings_pack::choking_algorithm,            0, 3 },
        { lt::settings_pack::seed_choking_algorithm,       0, 2 },
        { lt::settings_pack::suggest_mode,                 0, 1 },
        { lt::settings_pack::disk_write_mode,              0, 2 },
        { lt::settings_pack::num_optimistic_unchoke_slots, 0, std::numeric_limits<int>::max() },
        { lt::settings_pack::file_pool_size,               1, std::numeric_limits<int>::max() },
    };

    bool InRange(const lt::settings_pack& settings, const IntRange& range)
    {
        if (!settings.has_val(range.name)) { return true; }

        const int value = settings.get_int(range.name);
        return value >= range.min && value <= range.max;
    }
}
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
        lt::alert::error_notification
        | lt::alert::status_notification
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

std::optional<std::string> LibtorrentSettingsPack::Validate(const lt::settings_pack& settings)
{
    for (const auto& range : kIntRanges)
    {
        if (!InRange(settings, range))
        {
            return lt::name_for_setting(range.name);
        }
    }

    return std::nullopt;
}

void LibtorrentSettingsPack::Sanitize(lt::settings_pack& settings)
{
    for (const auto& range : kIntRanges)
    {
        if (!InRange(settings, range))
        {
            BOOST_LOG_TRIVIAL(warning)
                << "Ignoring invalid value " << settings.get_int(range.name)
                << " for setting " << lt::name_for_setting(range.name) << " - using the default";

            settings.clear(range.name);
        }
    }
}
