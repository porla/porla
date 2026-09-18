#pragma once

#include <nlohmann/json.hpp>
#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    class PoJson
    {
    public:
        static sol::object ToLua(sol::this_state ts, const nlohmann::json& data, int depth);
        static nlohmann::json ToJson(sol::this_state ts, const sol::object& data, int depth);

        static char* NullSentinel();
        static void TagArray(sol::table& t);
    };
}
