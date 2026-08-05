#pragma once

#include <sol/sol.hpp>

#define DECLARE_GLOBAL_OBJECT_IMPL(name) \
    class name                \
    {                         \
    public:                   \
        static sol::object Build(sol::state& lua); \
    };

namespace porla::Lua::Globals
{
    DECLARE_GLOBAL_OBJECT_IMPL(Cron)
    DECLARE_GLOBAL_OBJECT_IMPL(Http)
    DECLARE_GLOBAL_OBJECT_IMPL(Sleep)
}
