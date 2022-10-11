#include "settingspack.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/fingerprint.hpp>
#include <libtorrent/session.hpp>

namespace lt = libtorrent;
using porla::SettingsPack;

lt::settings_pack SettingsPack::Load(const toml::table &cfg)
{
    const std::string base = cfg["libtorrent"]["base"].value_or("default");

    auto pack = base == "min_memory_usage"
        ? lt::min_memory_usage()
        : base == "high_performance_seed"
            ? lt::high_performance_seed()
            : lt::default_settings();

    // Apply default settings here. These can be overwritten by settings from the
    // config file.
    pack.set_str(lt::settings_pack::user_agent, "porla/1.0");

    if (cfg.contains("proxy"))
    {
        if (auto host = cfg["proxy"]["host"].value<std::string>())
            pack.set_str(lt::settings_pack::proxy_hostname, *host);

        if (auto port = cfg["proxy"]["port"].value<int>())
            pack.set_int(lt::settings_pack::proxy_port, *port);

        if (auto type = cfg["proxy"]["type"].value<std::string>())
        {
            lt::settings_pack::proxy_type_t ltType = lt::settings_pack::none;

            if (type == "socks4")    ltType = lt::settings_pack::socks4;
            if (type == "socks5")    ltType = lt::settings_pack::socks5;
            if (type == "socks5_pw") ltType = lt::settings_pack::socks5_pw;
            if (type == "http")      ltType = lt::settings_pack::http;
            if (type == "http_pw")   ltType = lt::settings_pack::http_pw;
            if (type == "i2p_proxy") ltType = lt::settings_pack::i2p_proxy;

            pack.set_int(lt::settings_pack::proxy_type, ltType);
        }

        if (auto user = cfg["proxy"]["username"].value<std::string>())
            pack.set_str(lt::settings_pack::proxy_username, *user);

        if (auto pwd = cfg["proxy"]["password"].value<std::string>())
            pack.set_str(lt::settings_pack::proxy_password, *pwd);
    }

    for (int i = lt::settings_pack::bool_type_base; i < lt::settings_pack::max_bool_setting_internal; i++)
    {
        const char* name = lt::name_for_setting(i);
        if (strcmp(name, "") == 0) continue;

        if (auto val = cfg["libtorrent"][name])
        {
            if (!val.is_boolean())
            {
                BOOST_LOG_TRIVIAL(warning)
                    << "Invalid setting libtorrent." << name << " - not a bool (" << val.type() << ")";
                continue;
            }

            int key = lt::setting_by_name(name);
            pack.set_bool(key, *val.value<bool>());
        }
    }

    for (int i = lt::settings_pack::int_type_base; i < lt::settings_pack::max_int_setting_internal; i++)
    {
        const char* name = lt::name_for_setting(i);
        if (strcmp(name, "") == 0) continue;

        if (auto val = cfg["libtorrent"][name])
        {
            if (!val.is_number())
            {
                BOOST_LOG_TRIVIAL(warning)
                    << "Invalid setting libtorrent." << name << " - not a number (" << val.type() << ")";
                continue;
            }

            int key = lt::setting_by_name(name);
            pack.set_int(key, *val.value<int>());
        }
    }

    for (int i = lt::settings_pack::string_type_base; i < lt::settings_pack::max_string_setting_internal; i++)
    {
        const char* name = lt::name_for_setting(i);
        if (strcmp(name, "") == 0) continue;

        if (auto val = cfg["libtorrent"][name])
        {
            if (!val.is_string())
            {
                BOOST_LOG_TRIVIAL(warning)
                    << "Invalid setting libtorrent." << name << " - not a string (" << val.type() << ")";
                continue;
            }

            int key = lt::setting_by_name(name);
            pack.set_str(key, *val.value<std::string>());
        }
    }

    // Apply static settings here. These are always set after all other settings from
    // the config are applied, and cannot be overwritten by it.
    pack.set_str(lt::settings_pack::peer_fingerprint, lt::generate_fingerprint("PO", 0, 1));

    return pack;
}
