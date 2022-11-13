#include "sessionsettings.hpp"

#include <unordered_set>

#include <boost/log/trivial.hpp>

#include "../statement.hpp"

using porla::Data::Models::SessionSettings;

const std::unordered_set<std::string> SessionSettings::BlockedKeys =
{
    "peer_fingerprint",
    "user_agent"
};

void SessionSettings::Apply(sqlite3* db, libtorrent::settings_pack& settings)
{
    Statement::Prepare(db, "SELECT key,value FROM sessionsettings ORDER BY key ASC")
        .Step(
            [&settings](auto const& row)
            {
                auto const key = row.GetStdString(0);
                auto const val = row.GetStdString(1);

                nlohmann::json val_parsed;

                try
                {
                    val_parsed = nlohmann::json::parse(val);
                }
                catch (const std::exception& ex)
                {
                    BOOST_LOG_TRIVIAL(error) << "Failed to parse " << key << " as JSON: " << ex.what();
                    return SQLITE_OK;
                }

                if (BlockedKeys.contains(key))
                {
                    BOOST_LOG_TRIVIAL(warning) << "Will not apply key " << key << " to settings";
                    return SQLITE_OK;
                }

                int type = lt::setting_by_name(key);

                if (type == -1)
                {
                    BOOST_LOG_TRIVIAL(error) << "Key " << key << " is invalid";
                    return SQLITE_OK;
                }

                if ((type & lt::settings_pack::type_mask) == lt::settings_pack::bool_type_base)
                {
                    if (!val_parsed.is_boolean())
                    {
                        BOOST_LOG_TRIVIAL(error) << "Value for key " << key << " is not a boolean";
                        return SQLITE_OK;
                    }

                    settings.set_bool(type, val_parsed.get<bool>());
                }
                else if ((type & lt::settings_pack::type_mask) == lt::settings_pack::int_type_base)
                {
                    if (!val_parsed.is_number_integer())
                    {
                        BOOST_LOG_TRIVIAL(error) << "Value for key " << key << " is not an integer";
                        return SQLITE_OK;
                    }

                    settings.set_int(type, val_parsed.get<int>());
                }
                else if ((type & lt::settings_pack::type_mask) == lt::settings_pack::string_type_base)
                {
                    if (!val_parsed.is_string())
                    {
                        BOOST_LOG_TRIVIAL(error) << "Value for key " << key << " is not a string";
                        return SQLITE_OK;
                    }

                    settings.set_str(type, val_parsed.get<std::string>());
                }

                return SQLITE_OK;
            });
}

void SessionSettings::Update(sqlite3 *db, const std::string &name, const nlohmann::json &value)
{
    if (BlockedKeys.contains(name))
    {
        BOOST_LOG_TRIVIAL(warning) << "Will not set key " << name << " in settings";
        return;
    }

    Statement::Prepare(db, "INSERT INTO sessionsettings (key, value) VALUES ($1, $2)"
                           "ON CONFLICT (key) DO UPDATE SET value = excluded.value;")
        .Bind(1, std::string_view(name))
        .Bind(2, std::string_view(value.dump()))
        .Execute();
}
