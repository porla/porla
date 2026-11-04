#pragma once

#include <concepts>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <nlohmann/json.hpp>

namespace porla::Json
{
    struct FieldError : std::runtime_error
    {
        FieldError(std::string field, const std::string& reason)
            : std::runtime_error(reason)
            , field(std::move(field))
        {
        }

        std::string field;
    };

    template <typename T>
    concept Integer = std::integral<T> && !std::same_as<T, bool>;

    // Reads a JSON integer into T. nlohmann casts any number to the target type, so 99999999999
    // read as an int keeps its low 32 bits and 1.9 becomes 1; refuse both instead.
    template <Integer T>
    T ReadInteger(const nlohmann::json& j, std::string field = {})
    {
        if (j.is_number_unsigned())
        {
            if (const auto v = j.get<std::uint64_t>(); std::in_range<T>(v)) { return static_cast<T>(v); }
        }
        else if (j.is_number_integer())
        {
            if (const auto v = j.get<std::int64_t>(); std::in_range<T>(v)) { return static_cast<T>(v); }
        }
        else if (j.is_number_float())
        {
            throw FieldError(std::move(field), "must be a whole number");
        }
        else
        {
            return j.get<T>();
        }

        throw FieldError(std::move(field), "must be between "
            + std::to_string(std::numeric_limits<T>::min()) + " and "
            + std::to_string(std::numeric_limits<T>::max()));
    }

    template <typename T>
    T Read(const nlohmann::json& j)
    {
        if constexpr (Integer<T>) { return ReadInteger<T>(j); }
        else                      { return j.get<T>(); }
    }

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
    try
    {
        if constexpr (is_patch<T>)
        {
            const auto it = j.find(key);

            value.present = it != j.end();
            value.value   = value.present && !it->is_null()
                ? std::optional(Read<typename decltype(value.value)::value_type>(*it))
                : std::nullopt;
        }
        else if constexpr (is_optional<T>)
        {
            const auto it = j.find(key);

            value = it != j.end() && !it->is_null()
                ? std::optional(Read<typename T::value_type>(*it))
                : std::nullopt;
        }
        else if constexpr (Integer<T>)
        {
            value = ReadInteger<T>(j.at(key));
        }
        else
        {
            j.at(key).get_to(value);
        }
    }
    catch (FieldError& ex)
    {
        ex.field = ex.field.empty() ? key : std::string(key) + "." + ex.field;
        throw;
    }
    catch (const nlohmann::json::exception& ex)
    {
        // 403: j.at(key) on a missing key
        if (ex.id == 403)
        {
            throw FieldError(key, "is required");
        }

        // drop nlohmann's "[json.exception.type_error.302] " prefix
        const std::string_view what = ex.what();
        const auto             end  = what.find("] ");

        throw FieldError(key, std::string(end == std::string_view::npos ? what : what.substr(end + 2)));
    }
    catch (const std::invalid_argument& ex)
    {
        throw FieldError(key, ex.what());
    }
}

#define PORLA_JSON_READ_FIELD(field) ::porla::Json::ReadField(nlohmann_json_j, #field, nlohmann_json_t.field);

#define PORLA_JSON_REQUEST(Type, ...) \
  inline void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { \
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(PORLA_JSON_READ_FIELD, __VA_ARGS__)) }

#define PORLA_JSON_NO_PARAMS(Type) \
  inline void from_json(const nlohmann::json&, Type&) {}
