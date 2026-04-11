#include "../types.hpp"

#include <nlohmann/json.hpp>

using porla::Lua::Types::Json;

static sol::object json_to_lua(sol::this_state L, const nlohmann::json& j)
{
    switch (j.type())
    {
        case nlohmann::json::value_t::null:
            return sol::lua_nil;

        case nlohmann::json::value_t::boolean:
            return sol::make_object(L, j.get<bool>());

        case nlohmann::json::value_t::number_integer:
            return sol::make_object(L, j.get<int64_t>());

        case nlohmann::json::value_t::number_unsigned:
            return sol::make_object(L, j.get<uint64_t>());

        case nlohmann::json::value_t::number_float:
            return sol::make_object(L, j.get<double>());

        case nlohmann::json::value_t::string:
            return sol::make_object(L, j.get<std::string>());

        case nlohmann::json::value_t::array:
        {
            sol::table tbl(L, sol::create);

            for (size_t i = 0; i < j.size(); i++)
            {
                tbl[i + 1] = json_to_lua(L, j[i]);
            }

            return tbl;
        }

        case nlohmann::json::value_t::object:
        {
            sol::table tbl(L, sol::create);

            for (auto& [key, val] : j.items())
            {
                tbl[key] = json_to_lua(L, val);
            }

            return tbl;
        }

        default:
            return sol::lua_nil;
    }
}

void Json::Register(sol::state& lua)
{
    sol::table json = lua["json"].valid()
        ? lua["json"].get<sol::table>()
        : lua.create_named_table("json");

    json["parse"] = [](sol::this_state L, const std::string& input) -> std::tuple<sol::object, sol::object>
    {
        try
        {
            const auto parsed_json = nlohmann::json::parse(input);
            return { json_to_lua(L, parsed_json), sol::lua_nil };
        }
        catch (const nlohmann::json::parse_error& e)
        {
            return { sol::lua_nil, sol::make_object(L, std::string(e.what())) };
        }
    };
}
