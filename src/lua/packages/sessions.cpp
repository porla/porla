#include "../packages.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/session_stats.hpp>

#include "../plugin.hpp"
#include "../../sessions.hpp"
#include "../../torrentclientdata.hpp"
#include "../../utils/ltsettings.hpp"

using porla::Lua::Packages::Sessions;
using porla::Lua::PluginLoadOptions;
using porla::TorrentClientData;
using porla::Utils::LibtorrentSettingsPack;

class SessionsIter
{
public:
    explicit SessionsIter(std::map<int, std::shared_ptr<porla::Sessions::SessionState>>::const_iterator begin)
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
    std::map<int, std::shared_ptr<porla::Sessions::SessionState>>::const_iterator m_iter;
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

    auto session_type = lua.new_usertype<porla::Sessions::SessionState>(
        "lt.session",
        sol::no_constructor,
        "name",           sol::readonly(&porla::Sessions::SessionState::name),
        "add_torrent",    [](const std::shared_ptr<porla::Sessions::SessionState>& state, lt::add_torrent_params& params)
        {
            params.userdata.get<TorrentClientData>()->state = state;
            state->session->async_add_torrent(params);
        },
        "apply_settings", [](const porla::Sessions::SessionState& state, lt::settings_pack& sp)
        {
            LibtorrentSettingsPack::UpdateStatic(sp);
            state.session->apply_settings(sp);
        },
        "settings",       [](const porla::Sessions::SessionState& state) { return state.session->get_settings(); }
    );

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
            const auto& sessions = options.sessions.All();
            const auto& session = std::find_if(
                sessions.begin(),
                sessions.end(),
                [&name](const auto& iter)
                {
                    return iter.second->name == name;
                });

            return session == sessions.end()
                ? nullptr
                : session->second;
        };

        sessions["list"] = [](sol::this_state s)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const PluginLoadOptions&>();
            return SessionsIter(options.sessions.All().begin());
        };

        sessions["metrics"] = [](sol::this_state s)
        {
            sol::state_view lua{s};
            sol::table metrics = lua.create_table();

            for (const auto& metric : lt::session_stats_metrics())
            {
                metrics[metric.name] = metric.value_index;
            }

            return metrics;
        };

        return sessions;
    };
}
