#include "sessionssettingsset.hpp"

#include <boost/log/trivial.hpp>

#include "../../sessions.hpp"
#include "../../data/models/sessions.hpp"

using porla::Methods::SessionsSettingsSet;
using porla::Methods::SessionsSettingsSetReq;
using porla::Methods::SessionsSettingsSetRes;

static lt::settings_pack LoadFromJson(const std::map<std::string, nlohmann::json>& input)
{
    lt::settings_pack settings = lt::default_settings();

    for (const auto& [ key, value ] : input)
    {
        const int type = lt::setting_by_name(key.data());

        if (type == -1)
        {
            continue;
        }

        if ((type & lt::settings_pack::type_mask) == lt::settings_pack::bool_type_base)
        {
            if (!value.is_boolean())
            {
                BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key << " is not a boolean";
                continue;
            }

            settings.set_bool(type, value.get<bool>());
        }
        else if((type & lt::settings_pack::type_mask) == lt::settings_pack::int_type_base)
        {
            if (!value.is_number())
            {
                BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key << " is not an integer";
                continue;
            }

            settings.set_int(type, value.get<int>());
        }
        else if((type & lt::settings_pack::type_mask) == lt::settings_pack::string_type_base)
        {
            if (!value.is_string())
            {
                BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key << " is not a string";
                continue;
            }

            settings.set_str(type, value.get<std::string>());
        }
    }

    return settings;
}

SessionsSettingsSet::SessionsSettingsSet(sqlite3* db, porla::Sessions& sessions)
    : m_db(db),
    m_sessions(sessions)
{
}

void SessionsSettingsSet::Invoke(const SessionsSettingsSetReq& req, WriteCb<SessionsSettingsSetRes> cb)
{
    const auto& state = req.name.has_value()
        ? m_sessions.Get(req.name.value())
        : m_sessions.Default();

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    porla::Data::Models::Sessions::Update(
        m_db,
        state->name,
        LoadFromJson(req.settings));

    BOOST_LOG_TRIVIAL(info) << "Session settings for " << state->name << " updated";

    auto settings = porla::Data::Models::Sessions::GetByName(m_db, state->name);

    if (settings.has_value())
    {
        state->session->apply_settings(settings.value().params.settings);
    }

    cb.Ok(SessionsSettingsSetRes{});
}
