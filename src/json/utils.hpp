#pragma once

#include <nlohmann/json.hpp>

namespace porla::Json
{
    template <typename T>
    constexpr bool is_optional = false;

    template <typename T>
    constexpr bool is_optional<std::optional<T>> = true;

    // A missing or null key reads as std::nullopt; any other key is required.
    template <typename T>
    void ReadField(const nlohmann::json& j, const char* key, T& value)
    {
        if constexpr (is_optional<T>)
        {
            const auto it = j.find(key);

            value = it != j.end() && !it->is_null()
                ? std::optional(it->template get<typename T::value_type>())
                : std::nullopt;
        }
        else
        {
            j.at(key).get_to(value);
        }
    }
}

#define PORLA_JSON_READ_FIELD(field) ::porla::Json::ReadField(nlohmann_json_j, #field, nlohmann_json_t.field);

#define PORLA_JSON_REQUEST(Type, ...) \
  inline void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { \
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(PORLA_JSON_READ_FIELD, __VA_ARGS__)) }

#define PORLA_JSON_NO_PARAMS(Type) \
  inline void from_json(const nlohmann::json&, Type&) {}
