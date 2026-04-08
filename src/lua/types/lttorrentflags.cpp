#include "../types.hpp"

#include <libtorrent/torrent_flags.hpp>

using porla::Lua::Types::LtTorrentFlags;

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

void LtTorrentFlags::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["torrent_flags_t"] = lua.new_usertype<lt::torrent_flags_t>(
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
        });

    sol::table flags = lt.get_or("torrent_flags", lua.create_named_table("torrent_flags"));

    flags["seed_mode"]             = sol::var(lt::torrent_flags::seed_mode);
    flags["upload_mode"]           = sol::var(lt::torrent_flags::upload_mode);
    flags["share_mode"]            = sol::var(lt::torrent_flags::share_mode);
    flags["apply_ip_filter"]       = sol::var(lt::torrent_flags::apply_ip_filter);
    flags["paused"]                = sol::var(lt::torrent_flags::paused);
    flags["auto_managed"]          = sol::var(lt::torrent_flags::auto_managed);
    flags["duplicate_is_error"]    = sol::var(lt::torrent_flags::duplicate_is_error);
    flags["update_subscribe"]      = sol::var(lt::torrent_flags::update_subscribe);
    flags["super_seeding"]         = sol::var(lt::torrent_flags::super_seeding);
    flags["sequential_download"]   = sol::var(lt::torrent_flags::sequential_download);
    flags["stop_when_ready"]       = sol::var(lt::torrent_flags::stop_when_ready);
    flags["override_trackers"]     = sol::var(lt::torrent_flags::override_trackers);
    flags["override_web_seeds"]    = sol::var(lt::torrent_flags::override_web_seeds);
    flags["need_save_resume"]      = sol::var(lt::torrent_flags::need_save_resume);
    flags["disable_dht"]           = sol::var(lt::torrent_flags::disable_dht);
    flags["disable_lsd"]           = sol::var(lt::torrent_flags::disable_lsd);
    flags["disable_pex"]           = sol::var(lt::torrent_flags::disable_pex);
    flags["no_verify_files"]       = sol::var(lt::torrent_flags::no_verify_files);
    flags["default_dont_download"] = sol::var(lt::torrent_flags::default_dont_download);
    flags["i2p_torrent"]           = sol::var(lt::torrent_flags::i2p_torrent);
}