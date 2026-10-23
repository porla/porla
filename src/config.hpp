#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include <boost/program_options.hpp>
#include <sqlite3.h>

namespace fs = std::filesystem;

namespace porla
{
    class Config
    {
    public:
        std::optional<std::string> config_file;
        sqlite3*                   db;
        std::optional<std::string> db_file;
        std::optional<std::string> http_base_path;
        std::optional<std::string> http_host;
        std::optional<uint16_t>    http_port;
        std::string                secret_key;
        fs::path                   state_dir;

        static std::unique_ptr<Config> Load(const boost::program_options::variables_map& cmd);

        ~Config();

        Config(const Config&) = delete;
        Config(const Config&&) = delete;
        Config& operator=(const Config&) = delete;
        Config& operator=(Config&&) = delete;

    private:
        explicit Config() = default;
    };
}
