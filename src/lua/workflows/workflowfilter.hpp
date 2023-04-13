#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Workflows
{
    class WorkflowFilter
    {
    public:
        static bool Includes(const sol::object& filter, const sol::table& context);
    };
}
