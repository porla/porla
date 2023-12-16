#include "../packages.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/magnet_uri.hpp>

#include "../plugin.hpp"
#include "../../sessions.hpp"
#include "../../torrentclientdata.hpp"

using porla::Lua::Packages::Sessions;
using porla::Lua::PluginLoadOptions;
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
        const auto options = lua.globals()["__load_opts"].get<const PluginLoadOptions&>();

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

        const auto& [ th, _ ] = m_iter->second;
        std::advance(m_iter, 1);
        return th;
    }

private:
    const porla::Sessions::SessionState& m_state;
    std::map<lt::info_hash_t, std::tuple<lt::torrent_handle, lt::torrent_status>>::const_iterator m_iter;
};

void Sessions::Register(sol::state& lua)
{
    auto atp_type = lua.new_usertype<lt::add_torrent_params>(
        "AddTorrentParams",
        sol::factories([]()
        {
            auto atp = std::make_shared<lt::add_torrent_params>();
            atp->userdata = lt::client_data_t(new TorrentClientData());
            return atp;
        }),
        "from_magnet",     sol::factories([](const std::string& uri) -> std::pair<std::shared_ptr<lt::add_torrent_params>, std::optional<std::string>>
                           {
                               auto atp = std::make_shared<lt::add_torrent_params>();
                               atp->userdata = lt::client_data_t(new TorrentClientData());

                               lt::error_code ec;
                               lt::parse_magnet_uri(uri, *atp, ec);

                               if (ec)
                               {
                                   return std::pair(nullptr, ec.message());
                               }

                               return std::pair(atp, std::nullopt);
                           }),

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
        "upload_limit",    &lt::add_torrent_params::upload_limit,
        "userdata",        sol::property([](const lt::add_torrent_params& p) { return p.userdata.get<TorrentClientData>(); }));

    auto settings_pack_type = lua.new_usertype<lt::settings_pack>(
        "SettingsPack",
        // the lt settings packs
        "default",                     sol::factories([]() { return lt::default_settings(); }),
        "high_performance_seed",       sol::factories([]() { return lt::high_performance_seed(); }),
        "min_memory_usage",            sol::factories([]() { return lt::min_memory_usage(); }),
        // get & set
        sol::meta_function::index,     [](lt::settings_pack& sp, sol::stack_object key, sol::this_state L)
        {
            const auto key_str = key.as<std::optional<std::string>>();

            if (!key_str.has_value())
            {
                return sol::object(L, sol::in_place, sol::lua_nil);
            }

            const int type = lt::setting_by_name(key_str.value());

            if (type == -1)
            {
                BOOST_LOG_TRIVIAL(warning) << "Unknown setting: " << key_str.value();
                return sol::object(L, sol::in_place, sol::lua_nil);
            }

            if ((type & lt::settings_pack::type_mask) == lt::settings_pack::bool_type_base)
            {
                return sol::object(L, sol::in_place, sp.get_bool(type));
            }

            if ((type & lt::settings_pack::type_mask) == lt::settings_pack::int_type_base)
            {
                return sol::object(L, sol::in_place, sp.get_int(type));
            }

            if ((type & lt::settings_pack::type_mask) == lt::settings_pack::string_type_base)
            {
                return sol::object(L, sol::in_place, sp.get_str(type));
            }

            return sol::object(L, sol::in_place, sol::lua_nil);
        },
        sol::meta_function::new_index, [](lt::settings_pack& sp, sol::stack_object key, sol::stack_object value)
        {
            const auto key_str = key.as<std::optional<std::string>>();

            if (!key_str.has_value() || porla::Sessions::DisallowedSetting(key_str.value()))
            {
                return;
            }

            const int type = lt::setting_by_name(key_str.value());

            if (type == -1)
            {
                BOOST_LOG_TRIVIAL(warning) << "Unknown setting: " << key_str.value();
                return;
            }

            if ((type & lt::settings_pack::type_mask) == lt::settings_pack::bool_type_base && value.is<bool>())
            {
                sp.set_bool(type, value.as<bool>());
            }

            if ((type & lt::settings_pack::type_mask) == lt::settings_pack::int_type_base && value.is<int>())
            {
                sp.set_int(type, value.as<int>());
            }

            if ((type & lt::settings_pack::type_mask) == lt::settings_pack::string_type_base && value.is<std::string>())
            {
                sp.set_str(type, value.as<std::string>());
            }
        });

    auto session_type = lua.new_usertype<porla::Sessions::SessionState>(
        "porla.Session",
        sol::no_constructor,
        "apply_settings", [](const porla::Sessions::SessionState& state, const lt::settings_pack& sp) { state.session->apply_settings(sp); },
        "name",           sol::readonly(&porla::Sessions::SessionState::name),
        "settings",       [](const porla::Sessions::SessionState& state) { return state.session->get_settings(); });

    session_type["add_torrent"] = [](const std::shared_ptr<porla::Sessions::SessionState>& state, lt::add_torrent_params& params)
    {
        params.userdata.get<TorrentClientData>()->state = state;
        state->session->async_add_torrent(params);
    };

    session_type["find_torrent"] = [](const std::shared_ptr<porla::Sessions::SessionState>& state, const lt::info_hash_t& ih) -> std::optional<lt::torrent_handle>
    {
        const auto it = state->torrents.find(ih);

        if (it == state->torrents.end())
        {
            return std::nullopt;
        }

        const auto& [ th, _ ] = it->second;

        return th;
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
            const auto options = lua.globals()["__load_opts"].get<const PluginLoadOptions&>();
            return options.sessions.Get(name);
        };

        sessions["list"] = [](sol::this_state s)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const PluginLoadOptions&>();
            return SessionsIter(options.sessions.All().begin());
        };

        return sessions;
    };
}
