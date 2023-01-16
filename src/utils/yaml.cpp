#include "yaml.hpp"

using porla::Utils::Yaml;

nlohmann::json Yaml::ToJson(const YAML::Node& node)
{
    switch (node.Type())
    {
        case YAML::NodeType::Undefined:
        case YAML::NodeType::Null:
            return nullptr;
        case YAML::NodeType::Scalar:
        {
            try
            {
                return node.as<int>();
            } catch (const YAML::BadConversion&) {}

            try
            {
                return node.as<bool>();
            } catch (const YAML::BadConversion&) {}

            try
            {
                return node.as<float>();
            } catch (const YAML::BadConversion&) {}

            try
            {
                return node.as<std::string>();
            } catch (const YAML::BadConversion&) {}
        }
        case YAML::NodeType::Sequence:
        {
            nlohmann::json list;

            for (const auto& item : node)
            {
                list.emplace_back(ToJson(item));
            }

            return list;
        }
        case YAML::NodeType::Map:
        {
            nlohmann::json map;

            for (auto iterator = node.begin(); iterator != node.end(); ++iterator)
            {
                map[ToJson(iterator->first)] = ToJson(iterator->second);
            }

            return map;
        }
    }
}
