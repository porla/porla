#pragma once

#include <optional>

#include <nlohmann/json.hpp>

namespace porla::Json
{
    constexpr int kMaxDepth = 256;

    // Parses untrusted JSON, rejecting values nested deeper than max_depth. Deeper values would
    // later overflow the stack in code that walks them recursively (conversions, copies).
    inline nlohmann::json Parse(std::string_view text, int max_depth = kMaxDepth)
    {
        return nlohmann::json::parse(
            text,
            [max_depth](int depth, nlohmann::json::parse_event_t event, nlohmann::json&)
            {
                if (depth >= max_depth
                    && (event == nlohmann::json::parse_event_t::object_start
                        || event == nlohmann::json::parse_event_t::array_start))
                {
                    throw std::runtime_error("JSON nested deeper than " + std::to_string(max_depth) + " levels");
                }

                return true;
            });
    }

    template <typename T>
    constexpr bool is_optional = false;

    template <typename T>
    constexpr bool is_optional<std::optional<T>> = true;

    // a field of a patch: an absent key leaves the stored value, null clears it
    template <typename T>
    struct Patch
    {
        bool             present = false;
        std::optional<T> value;
    };

    template <typename T>
    constexpr bool is_patch = false;

    template <typename T>
    constexpr bool is_patch<Patch<T>> = true;

    // A missing or null key reads as std::nullopt; any other key is required.
    template <typename T>
    void ReadField(const nlohmann::json& j, const char* key, T& value)
    {
        if constexpr (is_patch<T>)
        {
            const auto it = j.find(key);

            value.present = it != j.end();
            value.value   = value.present && !it->is_null()
                ? std::optional(it->template get<typename decltype(value.value)::value_type>())
                : std::nullopt;
        }
        else if constexpr (is_optional<T>)
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
