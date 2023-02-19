#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::UserTypes
{
    struct Workflow
    {
        static sol::table Require(sol::this_state s);

        explicit Workflow(const sol::table& args);

        std::vector<sol::object> Actions();
        std::string              On();
        bool                     ShouldExecute(const sol::table& ctx);

    private:
        std::vector<sol::object> m_actions;
        sol::function            m_condition;
        std::string              m_on;
    };
}
