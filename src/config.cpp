#include "config.hpp"

#include <charconv>
#include <filesystem>

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include "data/migrate.hpp"
#include "httphostguard.hpp"
#include "utils/string.hpp"
 
namespace fs = std::filesystem;

using porla::Config;

namespace
{
    uint16_t ParsePort(const char* name, std::string_view value)
    {
        uint16_t   port = 0;
        const auto end  = value.data() + value.size();
        const auto res  = std::from_chars(value.data(), end, port);

        if (value.empty() || res.ec != std::errc() || res.ptr != end || port == 0)
        {
            throw std::runtime_error(std::string(name) + " must be a port number (1-65535), got '" + std::string(value) + "'");
        }

        return port;
    }

    // "a.example, b.example" -> {"a.example", "b.example"}
    std::vector<std::string> ParseHosts(const std::string& value)
    {
        std::vector<std::string> hosts;

        for (auto host : porla::Utils::String::Split(value, ","))
        {
            host.erase(0, host.find_first_not_of(" \t"));
            host.erase(host.find_last_not_of(" \t") + 1);

            if (!host.empty())
            {
                hosts.push_back(std::move(host));
            }
        }

        return hosts;
    }
}

std::unique_ptr<Config> Config::Load(const boost::program_options::variables_map& cmd)
{
    const static std::vector<fs::path> config_file_search_paths =
    {
        fs::current_path() / "porla.toml",

        // Check $XDG_CONFIG_HOME/porla/porla.toml
        std::getenv("XDG_CONFIG_HOME")
            ? fs::path(std::getenv("XDG_CONFIG_HOME")) / "porla" / "porla.toml"
            : fs::path(),

        // Check $HOME/.config/porla/porla.toml
        std::getenv("HOME")
            ? fs::path(std::getenv("HOME")) / ".config" / "porla" / "porla.toml"
            : fs::path(),

        // Check $HOME/.config/porla.toml
        std::getenv("HOME")
            ? fs::path(std::getenv("HOME")) / ".config" / "porla.toml"
            : fs::path(),

        "/etc/porla/porla.toml",
        "/etc/porla.toml"
    };

    auto cfg = std::unique_ptr<Config>(new Config());
    cfg->state_dir = fs::absolute(fs::current_path());

    // Check default locations for a config file.
    for (auto const& path : config_file_search_paths)
    {
        // Skip empty paths.
        if (path == fs::path()) continue;

        std::error_code ec;
        bool regular_file = fs::is_regular_file(path, ec);

        if (ec || !regular_file)
        {
            continue;
        }

        cfg->config_file = path;

        break;
    }

    if (auto val = std::getenv("PORLA_CONFIG_FILE"))           cfg->config_file        = val;
    if (auto val = std::getenv("PORLA_DB"))                    cfg->db_file            = val;
    if (auto val = std::getenv("PORLA_HTTP_ALLOWED_HOSTS"))    cfg->http_allowed_hosts = ParseHosts(val);
    if (auto val = std::getenv("PORLA_HTTP_BASE_PATH"))        cfg->http_base_path     = val;
    if (auto val = std::getenv("PORLA_HTTP_HOST"))             cfg->http_host          = val;
    if (auto val = std::getenv("PORLA_HTTP_PORT"))             cfg->http_port          = ParsePort("PORLA_HTTP_PORT", val);
    if (auto val = std::getenv("PORLA_STATE_DIR"))             cfg->state_dir          = val;

    // Logins are server-side sessions now - nothing is signed, so there is no secret key.
    if (std::getenv("PORLA_SECRET_KEY") != nullptr)
    {
        BOOST_LOG_TRIVIAL(warning) << "PORLA_SECRET_KEY is no longer used and can be removed";
    }

    if (cmd.count("config-file"))
    {
        cfg->config_file = cmd["config-file"].as<std::string>();
    }

    bool config_file_found = false;

    if (cfg->config_file)
    {
        std::error_code ec;
        config_file_found = fs::is_regular_file(*cfg->config_file, ec);

        BOOST_LOG_TRIVIAL(warning)
            << "Config file " << fs::absolute(*cfg->config_file, ec)
            << (config_file_found ? " is ignored" : " does not exist")
            << " - Porla no longer reads config files. Use flags or PORLA_* environment variables instead.";
    }

    if (cmd.count("db"))                    cfg->db_file            = cmd["db"].as<std::string>();
    if (cmd.count("http-allowed-hosts"))    cfg->http_allowed_hosts = ParseHosts(cmd["http-allowed-hosts"].as<std::string>());
    if (cmd.count("http-base-path"))        cfg->http_base_path     = cmd["http-base-path"].as<std::string>();
    if (cmd.count("http-host"))             cfg->http_host          = cmd["http-host"].as<std::string>();
    if (cmd.count("http-port"))             cfg->http_port          = ParsePort("--http-port", cmd["http-port"].as<std::string>());
    if (cmd.count("state-dir"))             cfg->state_dir          = cmd["state-dir"].as<std::string>();

    // If no db_file is set, default to a file in state_dir.
    if (!cfg->db_file.has_value())
    {
        const auto db_path = cfg->state_dir / "porla.sqlite";

        // Older Docker images kept their state in the working directory. Don't start over with an
        // empty database (and auth.init open to anyone) while the old one is still there.
        const auto old_db_path = fs::current_path() / "porla.sqlite";

        std::error_code ec;

        if (!fs::exists(db_path, ec)
            && fs::exists(old_db_path, ec)
            && fs::weakly_canonical(cfg->state_dir, ec) != fs::weakly_canonical(fs::current_path(), ec))
        {
            BOOST_LOG_TRIVIAL(fatal)
                << "Found a database at " << old_db_path << " but none in the state directory "
                << cfg->state_dir << ". Move porla.sqlite (and any porla.sqlite-wal/-shm next to it) into "
                << cfg->state_dir << ", or set --state-dir/PORLA_STATE_DIR to " << fs::current_path();

            throw std::runtime_error("Database not found in the state directory");
        }

        // The config file may have pointed db or state_dir somewhere else. Don't start over with an
        // empty database (and auth.init open to anyone) because it isn't read anymore.
        if (config_file_found && !fs::exists(db_path, ec) && !ec)
        {
            BOOST_LOG_TRIVIAL(fatal)
                << "Config file " << *cfg->config_file << " is no longer read, and the database " << db_path
                << " does not exist. Pass --db/--state-dir (or PORLA_DB/PORLA_STATE_DIR) to point at your"
                << " existing database, or remove the config file for a fresh install.";

            throw std::runtime_error("Refusing to create a new database while a config file is present");
        }

        cfg->db_file = db_path;
    }


    cfg->http_host_guard = std::make_shared<HttpHostGuard>(cfg->http_host.value_or("127.0.0.1"), cfg->http_allowed_hosts);

    if (cfg->http_host_guard->Enabled())
    {
        BOOST_LOG_TRIVIAL(info) << "HTTP requests are only answered for localhost names"
            << (cfg->http_allowed_hosts.empty() ? "" : " and the allowed hosts");
    }

    if (std::error_code ec; !fs::create_directories(cfg->state_dir, ec) && ec)
    {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to create state directory " << cfg->state_dir << ": " << ec.message();
        throw std::runtime_error("Failed to create state directory");
    }

    if (sqlite3_open(cfg->db_file.value_or("porla.sqlite").c_str(), &cfg->db) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to open SQLite connection: " << sqlite3_errmsg(cfg->db);

        // sqlite3_open returns a handle even when it fails - close it so ~Config doesn't use it
        sqlite3_close(cfg->db);
        cfg->db = nullptr;

        throw std::runtime_error("Failed to open SQLite connection");
    }

    if (sqlite3_exec(cfg->db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to enable WAL journal mode: " << sqlite3_errmsg(cfg->db);
        throw std::runtime_error("Failed to enable WAL journal mode");
    }

    if (sqlite3_exec(cfg->db, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(warning) << "Failed to set synchronous mode: " << sqlite3_errmsg(cfg->db);
    }

    if (sqlite3_busy_timeout(cfg->db, 10000) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(warning)
            << "Failed to set SQLite busy timeout: " << sqlite3_errmsg(cfg->db);
    }

    if (!porla::Data::Migrate(cfg->db))
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to run migrations";
        throw std::runtime_error("Failed to apply migrations");
    }

    return std::move(cfg);
}

Config::~Config()
{
    if (db != nullptr)
    {
        if (sqlite3_exec(db, "PRAGMA wal_checkpoint(TRUNCATE);", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to checkpoint WAL: " << sqlite3_errmsg(db);
        }

        if (sqlite3_close(db) != SQLITE_OK)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to close SQLite connection: " << sqlite3_errmsg(db);
        }
    }
}
