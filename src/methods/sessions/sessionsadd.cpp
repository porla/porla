#include "sessionsadd.hpp"

#include <boost/log/trivial.hpp>

#include "../../sessions.hpp"
#include "../../data/models/sessions.hpp"

using porla::Methods::SessionsAdd;
using porla::Methods::SessionsAddReq;
using porla::Methods::SessionsAddRes;

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

SessionsAdd::SessionsAdd(sqlite3* db, porla::Sessions& sessions)
    : m_db(db),
    m_sessions(sessions)
{
}

void SessionsAdd::Invoke(const SessionsAddReq& req, WriteCb<SessionsAddRes> cb)
{
    lt::settings_pack settings;

    porla::Data::Models::Sessions::Insert(
        m_db,
        req.name,
        LoadFromJson(req.settings));

    BOOST_LOG_TRIVIAL(info) << "Session " << req.name << " added";

    m_sessions.LoadByName(req.name);

    cb.Ok(SessionsAddRes{});
}
