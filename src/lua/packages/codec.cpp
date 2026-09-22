#include "codec.hpp"

#include <nlohmann/json.hpp>
#include <sodium.h>

#include "../types/pojson.hpp"

using porla::Lua::Packages::Codec;

namespace
{
    int ResolveBase64Variant(std::optional<int> variant)
    {
        const int v = variant.value_or(sodium_base64_VARIANT_ORIGINAL);

        switch (v)
        {
        case sodium_base64_VARIANT_ORIGINAL:
        case sodium_base64_VARIANT_ORIGINAL_NO_PADDING:
        case sodium_base64_VARIANT_URLSAFE:
        case sodium_base64_VARIANT_URLSAFE_NO_PADDING:
            return v;
        }

        throw sol::error("invalid base64 variant");
    }
}

sol::object Codec::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table b64 = lua.create_table();
    b64["VARIANT_ORIGINAL"]            = sodium_base64_VARIANT_ORIGINAL;
    b64["VARIANT_ORIGINAL_NO_PADDING"] = sodium_base64_VARIANT_ORIGINAL_NO_PADDING;
    b64["VARIANT_URLSAFE"]             = sodium_base64_VARIANT_URLSAFE;
    b64["VARIANT_URLSAFE_NO_PADDING"]  = sodium_base64_VARIANT_URLSAFE_NO_PADDING;

    b64.set_function("decode", [](const std::string& data, std::optional<int> variant)
    {
        const auto resolved_variant = ResolveBase64Variant(variant);

        std::string output;
        output.resize(data.size() / 4 * 3 + 3);

        std::size_t bin_len = 0;

        const int result = sodium_base642bin(
            reinterpret_cast<unsigned char*>(output.data()),
            output.size(),
            data.data(),
            data.size(),
            nullptr,
            &bin_len,
            nullptr,
            resolved_variant);

        if (result != 0)
        {
            throw sol::error("invalid base64");
        }

        output.resize(bin_len);

        return output;
    });

    b64.set_function("encode", [](const std::string& data, std::optional<int> variant)
    {
        const auto resolved_variant = ResolveBase64Variant(variant);
        const auto len              = sodium_base64_ENCODED_LEN(data.size(), resolved_variant);

        std::string output;
        output.resize(len);

        sodium_bin2base64(
            output.data(),
            output.size(),
            reinterpret_cast<const unsigned char*>(data.c_str()),
            data.size(),
            resolved_variant);

        output.resize(len - 1);

        return output;
    });

    sol::table json_tbl = lua.create_table();
    json_tbl.set_function("array", [](sol::this_state ts, sol::optional<sol::table> tbl)
    {
        sol::state_view lua(ts);
        sol::table t = tbl.value_or(lua.create_table());

        Types::PoJson::TagArray(t);

        return t;
    });

    json_tbl["null"] = sol::light(Types::PoJson::NullSentinel());

    json_tbl.set_function("decode", [](sol::this_state ts, const std::string& data)
        -> std::tuple<sol::object, std::optional<std::string>>
    {
        nlohmann::json parsed_json;

        try
        {
            parsed_json = nlohmann::json::parse(data);
            return std::make_tuple(Types::PoJson::ToLua(ts, parsed_json, 0), std::nullopt);
        }
        catch (const std::exception& ex)
        {
            return std::make_tuple(sol::lua_nil, ex.what());
        }
    });

    json_tbl.set_function("encode", [](sol::this_state ts, const sol::object& data)
    {
        try
        {
            return Types::PoJson::ToJson(ts, data, 0).dump();
        }
        catch (const nlohmann::json::type_error& e)
        {
            throw sol::error(std::string("json: ") + e.what() +
                            " (binary data must be base64-encoded first)");
        }
    });

    sol::table hex = lua.create_table();
    hex.set_function("decode", [](const std::string& data)
    {
        if (data.size() % 2 != 0) { throw sol::error("invalid hex length"); }

        std::string output;
        output.resize(data.size() / 2);

        std::size_t bin_len = 0;

        const int result = sodium_hex2bin(
            reinterpret_cast<unsigned char*>(output.data()),
            output.size(),
            data.data(),
            data.size(),
            nullptr,
            &bin_len,
            nullptr);

        if (result != 0)
        {
            throw sol::error("invalid hex string");
        }

        output.resize(bin_len);

        return output;
    });

    hex.set_function("encode", [](const std::string& data)
    {
        std::string output;
        output.resize(data.size() * 2 + 1);

        sodium_bin2hex(
            output.data(),
            output.size(),
            reinterpret_cast<const unsigned char*>(data.c_str()),
            data.size());

        output.resize(data.size() * 2);

        return output;
    });

    sol::table tbl = lua.create_table();
    tbl["base64"] = b64;
    tbl["hex"] = hex;
    tbl["json"] = json_tbl;

    return tbl;
}
