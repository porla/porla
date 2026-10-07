#pragma once

#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>

namespace porla
{
    struct Event
    {
        explicit Event(std::string name, nlohmann::json data = nlohmann::json::object())
            : name(std::move(name))
            , data(std::move(data))
        {
        }

        virtual ~Event() = default;

        std::string    name;
        nlohmann::json data;
        std::uint64_t  id = 0;
    };
}
