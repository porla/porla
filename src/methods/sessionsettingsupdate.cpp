#include "sessionsettingsupdate.hpp"

#include <boost/log/trivial.hpp>

#include "../data/models/sessionsettings.hpp"
#include "../session.hpp"

using porla::Data::Models::SessionSettings;
using porla::Methods::SessionSettingsUpdate;
using porla::Methods::SessionSettingsUpdateReq;
using porla::Methods::SessionSettingsUpdateRes;

SessionSettingsUpdate::SessionSettingsUpdate(porla::ISession& session, sqlite3* db)
    : m_session(session)
    , m_db(db)
{
}

void SessionSettingsUpdate::Invoke(const SessionSettingsUpdateReq& req, WriteCb<SessionSettingsUpdateRes> cb)
{
    auto current_settings = m_session.Settings();

    for (auto const& [key,value] : req.settings)
    {
        // Get the libtorrent type of this setting
        const int type = lt::setting_by_name(key);

        if (type == -1)
        {
            BOOST_LOG_TRIVIAL(warning) << "Invalid setting: " << key;
            continue;
        }

        if (SessionSettings::BlockedKeys.contains(key))
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

            current_settings.set_bool(type, value.get<bool>());
            SessionSettings::Update(m_db, key, value);
        }
        else if((type & lt::settings_pack::type_mask) == lt::settings_pack::int_type_base)
        {
            if (!value.is_number_integer())
            {
                BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key << " is not an integer";
                continue;
            }

            current_settings.set_int(type, value.get<int>());
            SessionSettings::Update(m_db, key, value);
        }
        else if((type & lt::settings_pack::type_mask) == lt::settings_pack::string_type_base)
        {
            if (!value.is_string())
            {
                BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key << " is not a string";
                continue;
            }

            current_settings.set_str(type, value.get<std::string>());
            SessionSettings::Update(m_db, key, value);
        }
    }

    m_session.ApplySettings(current_settings);

    cb.Ok(SessionSettingsUpdateRes{});
}
