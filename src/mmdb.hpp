#pragma once

#include <filesystem>
#include <memory>

#include <nlohmann/json.hpp>

namespace porla
{
    class Mmdb
    {
    public:
        static std::unique_ptr<Mmdb> Load(const std::filesystem::path& path);

        ~Mmdb();
        nlohmann::json Lookup(const std::string& key);

    private:
        class State;
        std::shared_ptr<State> m_state;
    };
}
