#include "sessionssettingslist.hpp"

#include "../../sessions.hpp"

using porla::Methods::SessionsSettingsList;
using porla::Methods::SessionsSettingsListReq;
using porla::Methods::SessionsSettingsListRes;

template <bool SSL> SessionsSettingsList<SSL>::SessionsSettingsList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

template <bool SSL> void SessionsSettingsList<SSL>::Invoke(const SessionsSettingsListReq &req, WriteCb<SessionsSettingsListRes, SSL> cb)
{
    SessionsSettingsListRes res;

    const auto& state = req.name.has_value()
        ? m_sessions.Get(req.name.value())
        : m_sessions.Default();

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    const auto& settings = state->session->get_settings();

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

namespace porla::Methods {
    template class SessionsSettingsList<true>;
    template class SessionsSettingsList<false>;
}
