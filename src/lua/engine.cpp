#include "engine.hpp"

#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>

using porla::Lua::Engine;
using porla::Lua::EngineOptions;

struct Engine::State
{
    sol::state lua;
};

struct LuaAction
{
};

struct ActionsExec : public LuaAction
{
    explicit ActionsExec(sol::table args)
    {
        BOOST_LOG_TRIVIAL(info) << args.size();
    }
};

struct ActionsSleep : public LuaAction
{
    explicit ActionsSleep(sol::table args)
    {
        BOOST_LOG_TRIVIAL(info) << args.size();
    }

    ~ActionsSleep()
    {
        BOOST_LOG_TRIVIAL(info) << "dtor actions sleep";
    }
};

struct ActionsTorrentMove : public LuaAction
{
    explicit ActionsTorrentMove(sol::table args)
    {
        BOOST_LOG_TRIVIAL(info) << args.size();
    }
};

class Workflow
{
public:
    explicit Workflow(const sol::table& args)
    {
        m_on = args["on"];

        BOOST_LOG_TRIVIAL(info) << sol::type_name(args.lua_state(), args["actions"].get_type());

        const sol::table& actions = args["actions"];

        for (const auto& item : actions)
        {
            BOOST_LOG_TRIVIAL(info) << sol::type_name(args.lua_state(), item.second.get_type());

            if (item.second.is<LuaAction*>())
            {
                auto f = item.second.as<LuaAction*>();
            }
        }
    }

    ~Workflow()
    {
        BOOST_LOG_TRIVIAL(info) << "Destroying workflow";
    }

    std::string GetOn() { return m_on; }

private:
    std::string m_on;
};

struct Engine::WorkflowInstance
{
    Workflow* workflow;
};


sol::table open_mylib(sol::this_state s)
{
    sol::state_view lua(s);

    sol::usertype<Workflow> workflow_type = lua.new_usertype<Workflow>(
        "Workflow",
        sol::constructors<Workflow(sol::table)>());

    workflow_type["on"] = sol::property(&Workflow::GetOn);

    sol::table module = lua.create_table();
    module["Workflow"] = workflow_type;
    module["Workflow"]["TorrentAdded"] = "TorrentAdded";

    return module["Workflow"];
}

sol::table OpenActionsExec(sol::this_state s)
{
    sol::state_view lua(s);

    sol::table module = lua.create_table();
    module.new_usertype<ActionsExec>(
            "Exec",
            sol::constructors<ActionsExec(sol::table)>(),
            sol::base_classes,
            sol::bases<LuaAction>());

    return module["Exec"];
}

sol::table OpenActionsSleep(sol::this_state s) {
    sol::state_view lua(s);

    sol::table module = lua.create_table();
    module.new_usertype<ActionsSleep>(
            "Sleep",
            sol::constructors<ActionsSleep(sol::table)>(),
            sol::base_classes,
            sol::bases<LuaAction>());

    return module["Sleep"];
}

sol::table OpenActionsTorrentMove(sol::this_state s) {
    sol::state_view lua(s);

    sol::table module = lua.create_table();
    module.new_usertype<ActionsTorrentMove>(
            "TorrentMove",
            sol::constructors<ActionsTorrentMove(sol::table)>(),
            sol::base_classes,
            sol::bases<LuaAction>());

    return module["TorrentMove"];
}

Engine::Engine(const EngineOptions& opts)
{
    m_state = std::make_unique<State>();
    m_state->lua.open_libraries(sol::lib::base, sol::lib::package);

    m_state->lua.require("porla.Workflow", sol::c_call<decltype(&open_mylib), &open_mylib>);
    m_state->lua.require("porla.actions.Exec", sol::c_call<decltype(&OpenActionsExec), &OpenActionsExec>);
    m_state->lua.require("porla.actions.Sleep", sol::c_call<decltype(&OpenActionsSleep), &OpenActionsSleep>);
    m_state->lua.require("porla.actions.TorrentMove", sol::c_call<decltype(&OpenActionsTorrentMove), &OpenActionsTorrentMove>);

    const auto workflow = m_state->lua.script_file("");

    m_workflow_instances.emplace_back(WorkflowInstance{
        .workflow = workflow.get<Workflow*>()
    });
}

Engine::~Engine() = default;
