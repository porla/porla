#include "workflow.hpp"

#include <boost/log/trivial.hpp>

#include "../workflows/actionbuilder.hpp"

using porla::Lua::UserTypes::Workflow;
using porla::Lua::Workflows::ActionBuilder;

sol::table Workflow::Require(sol::this_state s)
{
    sol::state_view lua(s);

    sol::usertype<Workflow> workflow_type = lua.new_usertype<Workflow>(
            "Workflow",
            sol::constructors<Workflow(sol::table)>());

    workflow_type["actions"] = sol::property(&Workflow::Actions);
    workflow_type["on"]      = sol::property(&Workflow::On);

    sol::table module = lua.create_table();
    module["Workflow"] = workflow_type;
    module["Workflow"]["TorrentAdded"] = "TorrentAdded";

    return module["Workflow"];
}

Workflow::Workflow(const sol::table& args)
{
    m_on = args["on"];

    if (args["condition"].is<std::function<bool(sol::table)>>())
    {
        m_condition = args["condition"];
    }

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

std::string Workflow::On()
{
    return m_on;
}

bool Workflow::ShouldExecute(const sol::table &ctx)
{
    // If no condition is set - always execute
    if (!m_condition)
    {
        return true;
    }

    return m_condition(ctx);
}
