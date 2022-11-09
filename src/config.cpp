#include "config.hpp"

#include <filesystem>
#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/fingerprint.hpp>
#include <libtorrent/session.hpp>
#include <toml++/toml.h>

#include "settingspack.hpp"

namespace fs = std::filesystem;
namespace lt = libtorrent;
namespace po = boost::program_options;

using porla::Config;

Config Config::Load(int argc, char **argv)
{
    const static std::vector<fs::path> config_file_search_paths =
    {
        fs::current_path() / "porla.toml",
        "/etc/porla/porla.toml",
        "/etc/porla.toml"
    };

    Config cfg = {};
    cfg.session_settings = lt::default_settings();

    // Check default locations for a config file.
    for (auto const& path : config_file_search_paths)
    {
        std::error_code ec;
        bool regular_file = fs::is_regular_file(path, ec);

        if (ec || !regular_file)
        {
            continue;
        }

        cfg.config_file = path;
    }

    if (auto val = std::getenv("PORLA_CONFIG_FILE"))           cfg.config_file     = val;
    if (auto val = std::getenv("PORLA_DB"))                    cfg.db              = val;
    if (auto val = std::getenv("PORLA_HTTP_AUTH_TOKEN"))       cfg.http_auth_token = val;
    if (auto val = std::getenv("PORLA_HTTP_HOST"))             cfg.http_host       = val;
    if (auto val = std::getenv("PORLA_HTTP_PORT"))             cfg.http_port       = std::stoi(val);
    if (auto val = std::getenv("PORLA_LOG_LEVEL"))             cfg.log_level       = val;
    if (auto val = std::getenv("PORLA_SESSION_SETTINGS_BASE"))
    {
        if (strcmp("default", val) == 0)               cfg.session_settings = lt::default_settings();
        if (strcmp("high_performance_seed", val) == 0) cfg.session_settings = lt::high_performance_seed();
        if (strcmp("min_memory_usage", val) == 0)      cfg.session_settings = lt::min_memory_usage();
    }
    if (auto val = std::getenv("PORLA_SUPERVISED_INTERVAL"))   cfg.supervised_interval   = std::stoi(val);
    if (auto val = std::getenv("PORLA_SUPERVISED_PID"))        cfg.supervised_pid        = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_DHT_STATS"))       cfg.timer_dht_stats       = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_SESSION_STATS"))   cfg.timer_session_stats   = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_TORRENT_UPDATES")) cfg.timer_torrent_updates = std::stoi(val);

    po::options_description desc("Allowed options");
    desc.add_options()
        ("config-file",           po::value<std::string>(), "Path to a porla.toml config file.")
        ("db",                    po::value<std::string>(), "Path to where the database will be stored.")
        ("help",                                            "Show usage")
        ("http-auth-token",       po::value<std::string>(), "The auth token to use for the HTTP server.")
        ("http-host",             po::value<std::string>(), "The host to listen on for HTTP traffic.")
        ("http-port",             po::value<uint16_t>(),    "The port to listen on for HTTP traffic.")
        ("log-level",             po::value<std::string>(), "The minimum log level to print.")
        ("session-settings-base", po::value<std::string>(), "The libtorrent base settings to use")
        ("supervised-interval",   po::value<int>(),         "The interval to use when checking the supervisor pid.")
        ("supervised-pid",        po::value<pid_t>(),       "A pid to a parent process. If this pid dies, we shut down.")
        ("timer-dht-stats",       po::value<int>(),         "The interval to use for the DHT stats updates.")
        ("timer-session-stats",   po::value<int>(),         "The interval to use for the session stats updates.")
        ("timer-torrent-updates", po::value<int>(),         "The interval to use for the torrent updates.")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc;
        exit(0); // TODO: Not ideal.
    }

    if (vm.count("config-file"))
    {
        cfg.config_file = vm["config-file"].as<std::string>();

        if (!fs::is_regular_file(cfg.config_file.value()))
        {
            BOOST_LOG_TRIVIAL(warning) << "User-specified config file does not exist: " << cfg.config_file.value();
        }
    }

    // Apply configuration from the config file before we apply the command line args.
    if (cfg.config_file && fs::is_regular_file(cfg.config_file.value()))
    {
        std::ifstream config_file_data(cfg.config_file.value(), std::ios::binary);

        try
        {
            const toml::table config_file_tbl = toml::parse(config_file_data);

            if (auto val = config_file_tbl["db"].value<std::string>())
                cfg.db = *val;

            if (auto val = config_file_tbl["http"]["auth_token"].value<std::string>())
                cfg.http_auth_token = *val;

            if (auto val = config_file_tbl["http"]["host"].value<std::string>())
                cfg.http_host = *val;

            if (auto val = config_file_tbl["http"]["port"].value<uint16_t>())
                cfg.http_port = *val;

            if (const toml::array* interfaces = config_file_tbl["listen_interfaces"].as<toml::array>())
            {
                std::stringstream lt;

                for (auto&& item : *interfaces)
                {
                    if (const toml::array* interface = item.as<toml::array>())
                    {
                        if (interface->size() < 2)
                        {
                            BOOST_LOG_TRIVIAL(warning) << "Invalid number of listen interface elements";
                            continue;
                        }

                        auto addr = (*interface)[0].value<std::string>();
                        auto port = (*interface)[1].value<int64_t>();

                        if (!addr) { BOOST_LOG_TRIVIAL(warning) << "Invalid listen interface address"; continue; }
                        if (!port) { BOOST_LOG_TRIVIAL(warning) << "Invalid listen interface port"; continue; }

                        lt << "," << *addr << ":" << *port;
                    }
                }

                if (!lt.str().empty())
                {
                    cfg.session_settings.set_str(lt::settings_pack::listen_interfaces, lt.str().substr(1));
                }
            }

            if (auto val = config_file_tbl["log_level"].value<std::string>())
                cfg.log_level = *val;

            // Load presets
            if (auto const* presets_tbl = config_file_tbl["presets"].as_table())
            {
                for (auto const [key,value] : *presets_tbl)
                {
                    if (!value.is_table())
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Preset '" << key << "' is not a TOML table";
                        continue;
                    }

                    const toml::table value_tbl = *value.as_table();

                    Preset p = {};

                    if (auto val = value_tbl["download_limit"].value<int>())
                        p.download_limit = *val;

                    if (auto val = value_tbl["max_connections"].value<int>())
                        p.max_connections = *val;

                    if (auto val = value_tbl["max_uploads"].value<int>())
                        p.max_uploads = *val;

                    if (auto val = value_tbl["save_path"].value<std::string>())
                        p.save_path = *val;

                    if (auto val = value_tbl["storage_mode"].value<std::string>())
                    {
                        if (strcmp(val->c_str(), "allocate") == 0) p.storage_mode = lt::storage_mode_allocate;
                        if (strcmp(val->c_str(), "sparse") == 0)   p.storage_mode = lt::storage_mode_sparse;
                    }

                    if (auto val = value_tbl["upload_limit"].value<int>())
                        p.upload_limit = *val;

                    cfg.presets.insert({ key.data(), std::move(p) });
                }
            }

            if (config_file_tbl.contains("proxy"))
            {
                if (auto host = config_file_tbl["proxy"]["host"].value<std::string>())
                    cfg.session_settings.set_str(lt::settings_pack::proxy_hostname, *host);

                if (auto port = config_file_tbl["proxy"]["port"].value<int>())
                    cfg.session_settings.set_int(lt::settings_pack::proxy_port, *port);

                if (auto type = config_file_tbl["proxy"]["type"].value<std::string>())
                {
                    lt::settings_pack::proxy_type_t ltType = lt::settings_pack::none;

                    if (type == "socks4")    ltType = lt::settings_pack::socks4;
                    if (type == "socks5")    ltType = lt::settings_pack::socks5;
                    if (type == "socks5_pw") ltType = lt::settings_pack::socks5_pw;
                    if (type == "http")      ltType = lt::settings_pack::http;
                    if (type == "http_pw")   ltType = lt::settings_pack::http_pw;
                    if (type == "i2p_proxy") ltType = lt::settings_pack::i2p_proxy;

                    cfg.session_settings.set_int(lt::settings_pack::proxy_type, ltType);
                }

                if (auto user = config_file_tbl["proxy"]["username"].value<std::string>())
                    cfg.session_settings.set_str(lt::settings_pack::proxy_username, *user);

                if (auto pwd = config_file_tbl["proxy"]["password"].value<std::string>())
                    cfg.session_settings.set_str(lt::settings_pack::proxy_password, *pwd);

                if (auto hostnames = config_file_tbl["proxy"]["hostnames"].value<bool>())
                    cfg.session_settings.set_bool(lt::settings_pack::proxy_hostnames, *hostnames);

                if (auto peerConns = config_file_tbl["proxy"]["peer_connections"].value<bool>())
                    cfg.session_settings.set_bool(lt::settings_pack::proxy_peer_connections, *peerConns);

                if (auto trackerConns = config_file_tbl["proxy"]["tracker_connections"].value<bool>())
                    cfg.session_settings.set_bool(lt::settings_pack::proxy_tracker_connections, *trackerConns);
            }

            if (auto val = config_file_tbl["session_settings"]["extensions"].as_array())
            {
                std::vector<lt_plugin> extensions;

                for (auto const& item : *val)
                {
                    if (auto const item_value = item.value<std::string>())
                    {
                        if (*item_value == "smart_ban")
                            extensions.emplace_back(&lt::create_smart_ban_plugin);

                        if (*item_value == "ut_metadata")
                            extensions.emplace_back(&lt::create_ut_metadata_plugin);

                        if (*item_value == "ut_pex")
                            extensions.emplace_back(&lt::create_ut_pex_plugin);
                    }
                    else
                    {
                        BOOST_LOG_TRIVIAL(warning)
                            << "Item in session_extension array is not a string (" << item.type() << ")";
                    }
                }

                cfg.session_extensions = extensions;
            }

            if (auto val = config_file_tbl["session_settings"]["base"].value<std::string>())
            {
                if (*val == "default")               cfg.session_settings = lt::default_settings();
                if (*val == "high_performance_seed") cfg.session_settings = lt::high_performance_seed();
                if (*val == "min_memory_usage")      cfg.session_settings = lt::min_memory_usage();
            }

            if (auto tbl = config_file_tbl["session_settings"].as_table())
            {
                SettingsPack::Apply(*tbl, cfg.session_settings);
            }

            if (auto val = config_file_tbl["timer"]["dht_stats"].value<int>())
                cfg.timer_dht_stats = *val;

            if (auto val = config_file_tbl["timer"]["session_stats"].value<int>())
                cfg.timer_session_stats = *val;

            if (auto val = config_file_tbl["timer"]["torrent_updates"].value<int>())
                cfg.timer_torrent_updates = *val;
        }
        catch (const toml::parse_error& err)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse config file '" << cfg.config_file.value() << "': " << err;
        }
    }

    if (vm.count("db"))                    cfg.db                    = vm["db"].as<std::string>();
    if (vm.count("http-auth-token"))       cfg.http_auth_token       = vm["http-auth-token"].as<std::string>();
    if (vm.count("http-host"))             cfg.http_host             = vm["http-host"].as<std::string>();
    if (vm.count("http-port"))             cfg.http_port             = vm["http-port"].as<uint16_t>();
    if (vm.count("log-level"))             cfg.log_level             = vm["log-level"].as<std::string>();
    if (vm.count("session-settings-base"))
    {
        auto val = vm["session-settings-base"].as<std::string>();

        if (val == "default")               cfg.session_settings = lt::default_settings();
        if (val == "high_performance_seed") cfg.session_settings = lt::high_performance_seed();
        if (val == "min_memory_usage")      cfg.session_settings = lt::min_memory_usage();
    }
    if (vm.count("supervised-interval"))   cfg.supervised_interval   = vm["supervised-interval"].as<int>();
    if (vm.count("supervised-pid"))        cfg.supervised_pid        = vm["supervised-pid"].as<pid_t>();
    if (vm.count("timer-dht-stats"))       cfg.timer_dht_stats       = vm["timer-dht-stats"].as<int>();
    if (vm.count("timer-session-stats"))   cfg.timer_session_stats   = vm["timer-session-stats"].as<pid_t>();
    if (vm.count("timer-torrent-updates")) cfg.timer_torrent_updates = vm["timer-torrent-updates"].as<pid_t>();

    // Apply static libtorrent settings here. These are always set after all other settings from
    // the config are applied, and cannot be overwritten by it.
    cfg.session_settings.set_str(lt::settings_pack::peer_fingerprint, lt::generate_fingerprint("PO", 0, 1));
    cfg.session_settings.set_str(lt::settings_pack::user_agent, "porla/1.0");

    return std::move(cfg);
}
