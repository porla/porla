#include "sessionsettingslist.hpp"

#include "../sessions.hpp"

using porla::Methods::SessionSettingsList;
using porla::Methods::SessionSettingsListReq;
using porla::Methods::SessionSettingsListRes;

SessionSettingsList::SessionSettingsList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionSettingsList::Invoke(const SessionSettingsListReq &req, WriteCb<SessionSettingsListRes> cb)
{
    SessionSettingsListRes res;

    auto const settings = m_sessions.Default()->session->get_settings();

    for (int i = lt::settings_pack::bool_type_base; i < lt::settings_pack::max_bool_setting_internal; i++)
    {
        const char *name = lt::name_for_setting(i);
        if (strcmp(name, "") == 0) continue;
        if (req.keys.has_value() && req.keys.value().find(name) == req.keys.value().end()) continue;

        res.settings.insert({name,settings.get_bool(i)});
    }

    for (int i = lt::settings_pack::int_type_base; i < lt::settings_pack::max_int_setting_internal; i++)
    {
        const char *name = lt::name_for_setting(i);
        if (strcmp(name, "") == 0) continue;
        if (req.keys.has_value() && req.keys.value().find(name) == req.keys.value().end()) continue;

        res.settings.insert({name,settings.get_int(i)});
    }

    for (int i = lt::settings_pack::string_type_base; i < lt::settings_pack::max_string_setting_internal; i++)
    {
        const char *name = lt::name_for_setting(i);
        if (strcmp(name, "") == 0) continue;
        if (req.keys.has_value() && req.keys.value().find(name) == req.keys.value().end()) continue;

        res.settings.insert({name,settings.get_str(i)});
    }

    cb.Ok(res);
}
