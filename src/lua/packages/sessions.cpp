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

static const std::unordered_map<std::string, lt::torrent_flags_t> FlagsLookup =
{
    { "seed_mode",           lt::torrent_flags::seed_mode },
    { "upload_mode",         lt::torrent_flags::upload_mode },
    { "share_mode",          lt::torrent_flags::share_mode },
    { "apply_ip_filter",     lt::torrent_flags::apply_ip_filter },
    { "paused",              lt::torrent_flags::paused },
    { "auto_managed",        lt::torrent_flags::auto_managed },
    { "duplicate_is_error",  lt::torrent_flags::duplicate_is_error },
    { "update_subscribe",    lt::torrent_flags::update_subscribe },
    { "super_seeding",       lt::torrent_flags::super_seeding },
    { "sequential_download", lt::torrent_flags::sequential_download },
    { "stop_when_ready",     lt::torrent_flags::stop_when_ready },
    { "override_trackers",   lt::torrent_flags::override_trackers },
    { "override_web_seeds",  lt::torrent_flags::override_web_seeds },
    { "need_save_resume",    lt::torrent_flags::need_save_resume },
    { "disable_dht",         lt::torrent_flags::disable_dht },
    { "disable_lsd",         lt::torrent_flags::disable_lsd },
    { "disable_pex",         lt::torrent_flags::disable_pex },
    { "no_verify_files",     lt::torrent_flags::no_verify_files },
    { "i2p_torrent",         lt::torrent_flags::i2p_torrent }
};

struct FilePrioritiesProxy
{
    lt::add_torrent_params& params;

    lt::download_priority_t get(int index)
    {
        return params.file_priorities.at(index - 1);
    }

    void set(int index, lt::download_priority_t prio)
    {
        int i = index - 1;

        if (i >= static_cast<int>(params.file_priorities.size()))
        {
            params.file_priorities.resize(i + 1, lt::default_priority);
        }

        params.file_priorities[i] = prio;
    }

    void append(lt::download_priority_t prio)
    {
        params.file_priorities.push_back(prio);
    }

    int size()
    {
        return static_cast<int>(params.file_priorities.size());
    }
};

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
    auto download_prio_type = lua.new_usertype<lt::download_priority_t>(
        "lt.download_priority_t",
        sol::no_constructor,
        sol::meta_function::equal_to, [](lt::download_priority_t a, lt::download_priority_t b) { return a == b; }
    );

    auto storage_mode_type = lua.new_usertype<lt::storage_mode_t>(
        "lt.storage_mode_t",
        sol::no_constructor,
        sol::meta_function::equal_to, [](lt::storage_mode_t a, lt::storage_mode_t b) { return a == b; }
    );

    storage_mode_type["allocate"] = sol::var(lt::storage_mode_t::storage_mode_allocate);
    storage_mode_type["sparse"] = sol::var(lt::storage_mode_t::storage_mode_sparse);

    auto torrent_flags_type = lua.new_usertype<lt::torrent_flags_t>(
        "lt.torrent_flags_t",
        sol::constructors<lt::torrent_flags_t()>(),

        sol::meta_function::bitwise_or,  [](lt::torrent_flags_t a, lt::torrent_flags_t b) { return a | b; },
        sol::meta_function::bitwise_and, [](lt::torrent_flags_t a, lt::torrent_flags_t b) { return a & b; },
        sol::meta_function::bitwise_not, [](lt::torrent_flags_t a) { return ~a; },
        sol::meta_function::equal_to,    [](lt::torrent_flags_t a, lt::torrent_flags_t b) { return a == b; },

        sol::meta_function::to_string, [](lt::torrent_flags_t f)
        {
            return "lt.torrent_flags_t{" + std::to_string(static_cast<std::uint64_t>(f)) + "}";
        },

        "describe", [](lt::torrent_flags_t& f)
        {
            std::vector<std::string> result;

            for (const auto& [name, flag] : FlagsLookup)
            {
                if ((f & flag) == flag) result.push_back(name);
            }

            return sol::as_table(std::move(result));
        }
    );

    torrent_flags_type["seed_mode"]             = sol::var(lt::torrent_flags::seed_mode);
    torrent_flags_type["upload_mode"]           = sol::var(lt::torrent_flags::upload_mode);
    torrent_flags_type["share_mode"]            = sol::var(lt::torrent_flags::share_mode);
    torrent_flags_type["apply_ip_filter"]       = sol::var(lt::torrent_flags::apply_ip_filter);
    torrent_flags_type["paused"]                = sol::var(lt::torrent_flags::paused);
    torrent_flags_type["auto_managed"]          = sol::var(lt::torrent_flags::auto_managed);
    torrent_flags_type["duplicate_is_error"]    = sol::var(lt::torrent_flags::duplicate_is_error);
    torrent_flags_type["update_subscribe"]      = sol::var(lt::torrent_flags::update_subscribe);
    torrent_flags_type["super_seeding"]         = sol::var(lt::torrent_flags::super_seeding);
    torrent_flags_type["sequential_download"]   = sol::var(lt::torrent_flags::sequential_download);
    torrent_flags_type["stop_when_ready"]       = sol::var(lt::torrent_flags::stop_when_ready);
    torrent_flags_type["override_trackers"]     = sol::var(lt::torrent_flags::override_trackers);
    torrent_flags_type["override_web_seeds"]    = sol::var(lt::torrent_flags::override_web_seeds);
    torrent_flags_type["need_save_resume"]      = sol::var(lt::torrent_flags::need_save_resume);
    torrent_flags_type["disable_dht"]           = sol::var(lt::torrent_flags::disable_dht);
    torrent_flags_type["disable_lsd"]           = sol::var(lt::torrent_flags::disable_lsd);
    torrent_flags_type["disable_pex"]           = sol::var(lt::torrent_flags::disable_pex);
    torrent_flags_type["no_verify_files"]       = sol::var(lt::torrent_flags::no_verify_files);
    torrent_flags_type["default_dont_download"] = sol::var(lt::torrent_flags::default_dont_download);
    torrent_flags_type["i2p_torrent"]           = sol::var(lt::torrent_flags::i2p_torrent);

    auto file_prio_proxy_type = lua.new_usertype<FilePrioritiesProxy>(
        "porla.internal.FilePrioritiesProxy",
        sol::no_constructor,
        sol::meta_function::index,     &FilePrioritiesProxy::get,
        sol::meta_function::new_index, &FilePrioritiesProxy::set,
        sol::meta_function::length,    &FilePrioritiesProxy::size,
        "append",                      &FilePrioritiesProxy::append
    );

    auto atp_type = lua.new_usertype<lt::add_torrent_params>(
        "lt.add_torrent_params",
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
        "file_priorities", sol::property([](lt::add_torrent_params& p) { return FilePrioritiesProxy{p}; }),
        "flags",           &lt::add_torrent_params::flags,
        "info_hash",       &lt::add_torrent_params::info_hashes,
        "max_connections", &lt::add_torrent_params::max_connections,
        "max_uploads",     &lt::add_torrent_params::max_uploads,
        "name",            &lt::add_torrent_params::name,
        "save_path",       &lt::add_torrent_params::save_path,
        "storage_mode",    sol::property(
            [](const lt::add_torrent_params& p) -> lt::storage_mode_t { return p.storage_mode; },
            [](lt::add_torrent_params& p, lt::storage_mode_t m) { p.storage_mode = m; }
        ),
        "ti",              &lt::add_torrent_params::ti,
        "tracker_tiers",   &lt::add_torrent_params::tracker_tiers,
        "trackerid",       &lt::add_torrent_params::trackerid,
        "trackers",        &lt::add_torrent_params::trackers,
        "upload_limit",    &lt::add_torrent_params::upload_limit,
        "userdata",        sol::property([](const lt::add_torrent_params& p) { return p.userdata.get<TorrentClientData>(); }));

    auto settings_pack_type = lua.new_usertype<lt::settings_pack>(
        "lt.settings_pack",
        sol::constructors<lt::settings_pack()>(),

        "clear", sol::overload(
            [](lt::settings_pack& sp)           { sp.clear(); },
            [](lt::settings_pack& sp, int name) { sp.clear(name); }
        ),

        "has_val", &lt::settings_pack::has_val,

        // get & set
        "get_bool", [](const lt::settings_pack& sp, int name) { return sp.get_bool(name); },
        "get_int",  [](const lt::settings_pack& sp, int name) { return sp.get_int(name); },
        "get_str",  [](const lt::settings_pack& sp, int name) { return sp.get_str(name); },

        "set_bool", [](lt::settings_pack& sp, int name, bool val)        { sp.set_bool(name, val); },
        "set_int",  [](lt::settings_pack& sp, int name, int val)         { sp.set_int(name, val); },
        "set_str",  [](lt::settings_pack& sp, int name, std::string val) { sp.set_str(name, val); }
    );

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

    auto lt_table = lua.create_named_table("lt");
    lt_table["add_torrent_params"] = atp_type;

    lt_table["download_priority_t"] = download_prio_type;
    lt_table["dont_download"]       = sol::var(lt::dont_download);
    lt_table["default_priority"]    = sol::var(lt::default_priority);
    lt_table["low_priority"]        = sol::var(lt::low_priority);
    lt_table["top_priority"]        = sol::var(lt::top_priority);

    // the lt settings packs
    lt_table["default"]               = sol::factories([]() { return lt::default_settings(); });
    lt_table["high_performance_seed"] = sol::factories([]() { return lt::high_performance_seed(); });
    lt_table["min_memory_usage"]      = sol::factories([]() { return lt::min_memory_usage(); });

    lt_table["setting_by_name"] = [](const std::string& name) -> sol::optional<int>
    {
        int val = lt::setting_by_name(name);
        if (val == -1) return sol::nullopt;
        return val;
    };

    lt_table["settings_pack"] = settings_pack_type;
    lt_table["storage_mode_t"] = storage_mode_type;
    lt_table["torrent_flags_t"] = torrent_flags_type;

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
