#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include "keyvalueevent.hpp"

namespace porla
{
    class Events;

    struct KeyValueOptions
    {
        sqlite3* db;
        Events&  events;
    };

    class KeyValue
    {
    public:
        explicit KeyValue(const KeyValueOptions& options);

        nlohmann::json Get(const std::string& key) const;

        void Set(const std::string& key, const nlohmann::json& value);
        void Set(const std::map<std::string, nlohmann::json>& values);

    private:
        KeyValueOptions m_options;
    };
}
