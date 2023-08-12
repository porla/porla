#include "sessions.hpp"

#include <boost/log/trivial.hpp>

#include "../plugins/plugin.hpp"
#include "../../sessions.hpp"
#include "../../torrentclientdata.hpp"

using porla::Lua::Packages::Sessions;
using porla::Lua::Plugins::PluginLoadOptions;
using porla::TorrentClientData;

class SessionsIter
{
public:
    explicit SessionsIter(std::map<std::string, std::shared_ptr<porla::Sessions::SessionState>>::const_iterator begin)
        : m_iter(begin)
    {
    }

    std::shared_ptr<porla::Sessions::SessionState> operator()(sol::this_state s)
    {
        sol::state_view lua{s};
        const auto options = lua.globals()["__load_opts"].get<const porla::Lua::Plugins::PluginLoadOptions&>();

        if (m_iter == options.sessions.All().end()) return nullptr;

        auto session = m_iter->second;
        std::advance(m_iter, 1);
        return session;
    }

private:
    std::map<std::string, std::shared_ptr<porla::Sessions::SessionState>>::const_iterator m_iter;
};

class TorrentsIter
{
public:
    explicit TorrentsIter(const porla::Sessions::SessionState& state)
        : m_state(state)
        , m_iter(state.torrents.begin())
    {
    }

    std::optional<lt::torrent_handle> operator()()
    {
        if (m_iter == m_state.torrents.end()) return std::nullopt;

        auto th = m_iter->second;
        std::advance(m_iter, 1);
        return th;
    }

private:
    const porla::Sessions::SessionState& m_state;
    std::map<lt::info_hash_t, lt::torrent_handle>::const_iterator m_iter;
};

void Sessions::Register(sol::state& lua)
{
    auto atp_type = lua.new_usertype<lt::add_torrent_params>(
        "AddTorrentParams",
        "download_limit",  &lt::add_torrent_params::download_limit,
        "file_priorities", &lt::add_torrent_params::file_priorities,
        // flags,
        "info_hash",       &lt::add_torrent_params::info_hashes,
        "max_connections", &lt::add_torrent_params::max_connections,
        "max_uploads",     &lt::add_torrent_params::max_uploads,
        "name",            &lt::add_torrent_params::name,
        "save_path",       &lt::add_torrent_params::save_path,
        // storage mode
        "ti",              &lt::add_torrent_params::ti,
        "tracker_tiers",   &lt::add_torrent_params::tracker_tiers,
        "trackerid",       &lt::add_torrent_params::trackerid,
        "trackers",        &lt::add_torrent_params::trackers,
        "upload_limit",    &lt::add_torrent_params::upload_limit);

    auto session_type = lua.new_usertype<porla::Sessions::SessionState>(
        "porla.Session",
        sol::no_constructor,
        "name", sol::readonly(&porla::Sessions::SessionState::name));

    session_type["add_torrent"] = [](const std::shared_ptr<porla::Sessions::SessionState>& state, lt::add_torrent_params& params)
    {
        params.userdata = lt::client_data_t(new TorrentClientData());
        params.userdata.get<TorrentClientData>()->state = state;

        // TODO: set category and tags

        state->session->async_add_torrent(params);
    };

    session_type["find_torrent"] = [](const std::shared_ptr<porla::Sessions::SessionState>& state, const lt::info_hash_t& ih) -> std::optional<lt::torrent_handle>
    {
        const auto it = state->torrents.find(ih);

        if (it == state->torrents.end())
        {
            return std::nullopt;
        }

        return it->second;
    };

    session_type["remove_torrent"] = [](const porla::Sessions::SessionState& state, const lt::torrent_handle& th, const sol::table& args)
    {
        bool remove_files = false;
        if (args["remove_files"].valid()) { remove_files = args["remove_files"].get<bool>(); }

        state.session->remove_torrent(th, remove_files ? lt::session::delete_files : lt::remove_flags_t{});
    };

    session_type["torrents"] = [](const porla::Sessions::SessionState& state)
    {
        return TorrentsIter(state);
    };

    lua["package"]["preload"]["sessions"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table sessions = lua.create_table();

        sessions["get"] = [](sol::this_state s, const std::string& name)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();
            return options.sessions.Get(name);
        };

        sessions["list"] = [](sol::this_state s)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();
            return SessionsIter(options.sessions.All().begin());
        };

        sessions["settings"] = lua.create_table();
        sessions["settings"]["get"] = [](sol::this_state s, const std::string& session) -> sol::reference
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();
            const auto state = options.sessions.Get(session);

            if (state == nullptr)
            {
                BOOST_LOG_TRIVIAL(warning) << "Could not find session " << session;
                return sol::nil;
            }

            const auto& session_settings = state->session->get_settings();

            sol::table settings = lua.create_table();

            for (int i = lt::settings_pack::bool_type_base; i < lt::settings_pack::max_bool_setting_internal; i++)
            {
                const char *name = lt::name_for_setting(i);
                if (strcmp(name, "") == 0) continue;
                settings[name] = session_settings.get_bool(i);
            }

            for (int i = lt::settings_pack::int_type_base; i < lt::settings_pack::max_int_setting_internal; i++)
            {
                const char *name = lt::name_for_setting(i);
                if (strcmp(name, "") == 0) continue;
                settings[name] = session_settings.get_int(i);
            }

            for (int i = lt::settings_pack::string_type_base; i < lt::settings_pack::max_string_setting_internal; i++)
            {
                const char *name = lt::name_for_setting(i);
                if (strcmp(name, "") == 0) continue;
                settings[name] = session_settings.get_str(i);
            }

            return settings;
        };

        sessions["settings"]["set"] = [](sol::this_state s, const std::string& session, const sol::table& settings)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();
            const auto state = options.sessions.Get(session);

            if (state == nullptr)
            {
                BOOST_LOG_TRIVIAL(warning) << "Could not find session " << session;
                return;
            }

            lt::settings_pack session_settings = state->session->get_settings();

            for (const auto& [ key, value ] : settings)
            {
                const auto key_str = key.as<std::string>();

                if (porla::Sessions::DisallowedSetting(key_str))
                {
                    BOOST_LOG_TRIVIAL(warning) << "Will not set setting " << key_str;
                    continue;
                }

                const int type = lt::setting_by_name(key_str);

                if (type == -1)
                {
                    BOOST_LOG_TRIVIAL(warning) << "Unknown setting: " << key_str;
                    continue;
                }

                if ((type & lt::settings_pack::type_mask) == lt::settings_pack::bool_type_base)
                {
                    if (!value.is<bool>())
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key_str << " is not a boolean";
                        continue;
                    }

                    session_settings.set_bool(type, value.as<bool>());
                }
                else if((type & lt::settings_pack::type_mask) == lt::settings_pack::int_type_base)
                {
                    if (!value.is<int>())
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key_str << " is not an integer";
                        continue;
                    }

                    session_settings.set_int(type, value.as<int>());
                }
                else if((type & lt::settings_pack::type_mask) == lt::settings_pack::string_type_base)
                {
                    if (!value.is<std::string>())
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key_str << " is not a string";
                        continue;
                    }

                    session_settings.set_str(type, value.as<std::string>());
                }
            }

            state->session->apply_settings(session_settings);
        };

        return sessions;
    };
}
