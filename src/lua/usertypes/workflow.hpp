#pragma once

#include <sol/sol.hpp>

#include "../workflows/triggerbuilder.hpp"

using porla::Lua::Workflows::TriggerBuilder;

namespace porla::Lua::UserTypes
{
    struct Workflow
    {
        static sol::table Require(sol::this_state s);

        explicit Workflow(const sol::table& args);

        std::vector<sol::object>   Actions();
        sol::object                Filter();
        Workflows::TriggerBuilder& TriggerBuilder();

    private:
        std::vector<sol::object> m_actions;
        sol::object              m_filter;
        sol::object              m_trigger;
    };
}
