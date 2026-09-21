#include "pojson.hpp"

using porla::Lua::Types::PoJson;

namespace
{
    constexpr int kMaxDepth = 100;

    sol::table ArrayMetatable(lua_State* L)
    {
        sol::state_view lua(L);

        sol::object existing = lua.registry()["porla.json.array_mt"];

        if (existing.valid())
        {
            return existing.as<sol::table>();
        }

        sol::table mt = lua.create_table();
        mt["__porla_json_array"] = true;

        lua.registry()["porla.json.array_mt"] = mt;

        return mt;
    }

    bool IsLuaInteger(const sol::object& o)
    {
        lua_State* L = o.lua_state();
        o.push(L);

        const bool result = lua_isinteger(L, -1) != 0;

        lua_pop(L, 1);

        return result;
    }

    bool IsTaggedAsArray(const sol::table& tbl)
    {
        sol::optional<sol::table> mt = tbl[sol::metatable_key];

        if (!mt) { return false; }

        sol::optional<bool> marker = (*mt)["__porla_json_array"];

        return marker.value_or(false);
    }

    nlohmann::json ConvertTable(sol::this_state ts, const sol::table& tbl, int depth)
    {
        bool         all_integer_keys = true;
        std::size_t  count            = 0;
        std::int64_t max_index        = 0;

        for (const auto& kv : tbl)
        {
            count++;

            if (kv.first.get_type() == sol::type::number && IsLuaInteger(kv.first))
            {
                const auto index = kv.first.as<std::int64_t>();

                if (index < 1) { all_integer_keys = false; }
                else if (index > max_index) { max_index = index; }
            }
            else
            {
                all_integer_keys = false;
            }
        }

        if (count == 0)
        {
            return IsTaggedAsArray(tbl)
                ? nlohmann::json::array()
                : nlohmann::json::object();
        }

        if (all_integer_keys)
        {
            if (static_cast<std::size_t>(max_index) != count)
            {
                throw sol::error("json: cannot encode sparse array");
            }

            auto out = nlohmann::json::array();

            for (std::int64_t i = 1; i <= max_index; i++)
            {
                out.push_back(PoJson::ToJson(ts, tbl[i], depth + 1));
            }

            return out;
        }

        auto out = nlohmann::json::object();

        for (const auto& kv : tbl)
        {
            if (kv.first.get_type() != sol::type::string)
            {
                throw sol::error("json: object keys must be strings");
            }

            out[kv.first.as<std::string>()] = PoJson::ToJson(ts, kv.second, depth + 1);
        }

        return out;
    }
}

sol::object PoJson::ToLua(sol::this_state ts, const nlohmann::json& data, int depth)
{
    if (depth > kMaxDepth)
    {
        throw sol::error("json: maximum nesting depth exceeded");
    }

    sol::state_view lua(ts);

    switch (data.type())
    {
    case nlohmann::json::value_t::null:
        return sol::object(lua, sol::in_place, NullSentinel());

    case nlohmann::json::value_t::boolean:
        return sol::make_object(lua, data.get<bool>());

    case nlohmann::json::value_t::number_integer:
        return sol::make_object(lua, data.get<std::int64_t>());

    case nlohmann::json::value_t::number_unsigned:
    {
        const auto value = data.get<std::uint64_t>();

        if (value > static_cast<std::uint64_t>(
                std::numeric_limits<std::int64_t>::max()))
        {
            return sol::make_object(lua, static_cast<double>(value));
        }

        return sol::make_object(lua, static_cast<std::int64_t>(value));
    }

    case nlohmann::json::value_t::number_float:
        return sol::make_object(lua, data.get<double>());

    case nlohmann::json::value_t::string:
        return sol::make_object(lua, data.get<std::string>());

    case nlohmann::json::value_t::array:
    {
        sol::table out = lua.create_table(
            static_cast<int>(data.size()), 0);

        std::int64_t index = 1;

        for (const auto& item : data)
        {
            out[index++] = ToLua(ts, item, depth + 1);
        }

        TagArray(out);

        return out;
    }

    case nlohmann::json::value_t::object:
    {
        sol::table out = lua.create_table(
            0, static_cast<int>(data.size()));

        for (const auto& [key, value] : data.items())
        {
            out[key] = ToLua(ts, value, depth + 1);
        }

        return out;
    }

    case nlohmann::json::value_t::binary:
        throw sol::error("json: cannot decode binary values");

    case nlohmann::json::value_t::discarded:
    default:
        break;
    }

    throw sol::error("json: unsupported value type");
}

nlohmann::json PoJson::ToJson(sol::this_state ts, const sol::object& data, int depth)
{
    if (depth > kMaxDepth)
    {
        throw sol::error("json: maximum nesting depth exceeded");
    }

    switch (data.get_type())
    {
        case sol::type::lua_nil:
        case sol::type::none:
            return nlohmann::json();

        case sol::type::boolean:
            return nlohmann::json(data.as<bool>());

        case sol::type::number:
        {
            if (IsLuaInteger(data))
            {
                return nlohmann::json(data.as<std::int64_t>());
            }

            const double value = data.as<double>();

            if (!std::isfinite(value))
            {
                throw sol::error("json: cannot encode NaN or infinity");
            }

            return nlohmann::json(value);
        }

        case sol::type::string:
            return nlohmann::json(data.as<std::string>());

        case sol::type::lightuserdata:
            if (data.as<sol::light<char>>().value() == NullSentinel())
            {
                return nlohmann::json();
            }

            throw sol::error("json: cannot encode userdata");

        case sol::type::table:
            return ConvertTable(ts, data.as<sol::table>(), depth);

        default:
            break;
    }

    throw sol::error(
        std::string("json: cannot encode value of type ") +
        sol::type_name(ts, data.get_type()));
}

char* PoJson::NullSentinel()
{
    static char sentinel = 0;
    return &sentinel;
}

void PoJson::TagArray(sol::table& tbl)
{
    tbl[sol::metatable_key] = ArrayMetatable(tbl.lua_state());
}
