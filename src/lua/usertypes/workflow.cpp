#include "workflow.hpp"

#include <boost/log/trivial.hpp>

#include "../workflows/actionbuilder.hpp"

using porla::Lua::UserTypes::Workflow;
using porla::Lua::Workflows::ActionBuilder;
using porla::Lua::Workflows::TriggerBuilder;

sol::table Workflow::Require(sol::this_state s)
{
    sol::state_view lua(s);

    sol::usertype<Workflow> workflow_type = lua.new_usertype<Workflow>(
            "Workflow",
            sol::constructors<Workflow(sol::table)>());

    sol::table module = lua.create_table();
    module["Workflow"] = workflow_type;

    return module["Workflow"];
}

Workflow::Workflow(const sol::table& args)
    : m_trigger(args["on"])
{
    const sol::table& actions = args["actions"];

    for (const auto& item : actions)
    {
        if (item.second.is<ActionBuilder*>())
        {
            m_actions.emplace_back(item.second);
        }
    }
}

std::vector<sol::object> Workflow::Actions()
{
    return m_actions;
}

TriggerBuilder& Workflow::TriggerBuilder()
{
    return m_trigger.as<porla::Lua::Workflows::TriggerBuilder&>();
}
