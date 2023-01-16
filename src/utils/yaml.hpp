#pragma once

#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

namespace porla::Utils
{
    class Yaml
    {
    public:
        static nlohmann::json ToJson(const YAML::Node& node);
    };
}
