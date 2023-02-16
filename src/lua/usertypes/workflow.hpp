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

    private:
        std::vector<sol::object> m_actions;
        std::string              m_on;
    };
}
