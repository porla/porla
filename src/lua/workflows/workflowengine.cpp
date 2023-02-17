#include "workflowengine.hpp"

#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>

#include "../usertypes/lttorrenthandle.hpp"
#include "../usertypes/workflow.hpp"
#include "../usertypes/workflowactionexec.hpp"
#include "../usertypes/workflowactionlog.hpp"
#include "../usertypes/workflowactionpushdiscord.hpp"
#include "../usertypes/workflowactionpushntfy.hpp"
#include "../usertypes/workflowactionsleep.hpp"
#include "../usertypes/workflowactiontorrentmove.hpp"

#include "action.hpp"
#include "actionbuilder.hpp"
#include "workflowrunner.hpp"

#include "../../session.hpp"

namespace fs = std::filesystem;

using porla::Lua::Workflows::WorkflowEngine;
using porla::Lua::Workflows::WorkflowEngineOptions;
using porla::Lua::UserTypes::Workflow;

using porla::Lua::UserTypes::LibtorrentTorrentHandle;
using porla::Lua::UserTypes::WorkflowActionExec;
using porla::Lua::UserTypes::WorkflowActionLog;
using porla::Lua::UserTypes::WorkflowActionPushDiscord;
using porla::Lua::UserTypes::WorkflowActionPushNtfy;
using porla::Lua::UserTypes::WorkflowActionSleep;
using porla::Lua::UserTypes::WorkflowActionTorrentMove;

using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilder;
using porla::Lua::Workflows::ActionBuilderOptions;
using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::WorkflowRunner;
using porla::Lua::Workflows::WorkflowRunnerOptions;

template<typename T>
sol::table OpenWorkflowActionT(sol::this_state s)
{
    sol::state_view lua(s);

    sol::table module = lua.create_table();
    module.new_usertype<T>(
        T::ShortName(),
        sol::constructors<T(sol::table)>(),
        sol::base_classes,
        sol::bases<ActionBuilder>());

    return module[T::ShortName()];
}

struct WorkflowEngine::State
{
    sol::state                  lua;
    boost::signals2::connection on_torrent_added;
    std::vector<sol::object>    workflows;
};

WorkflowEngine::WorkflowEngine(const WorkflowEngineOptions& opts)
    : m_opts(opts)
{
    m_state = std::make_unique<State>();
    m_state->lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);

    LibtorrentTorrentHandle::Register(m_state->lua);

    m_state->lua.require("porla.Workflow", sol::c_call<decltype(&Workflow::Require), &Workflow::Require>);

    m_state->lua.require("porla.actions.Exec",
                         sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionExec>),
                         &OpenWorkflowActionT<WorkflowActionExec>>);
    m_state->lua.require("porla.actions.Log",
                         sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionLog>),
                         &OpenWorkflowActionT<WorkflowActionLog>>);
    m_state->lua.require("porla.actions.PushDiscord",
                         sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionPushDiscord>),
                         &OpenWorkflowActionT<WorkflowActionPushDiscord>>);
    m_state->lua.require("porla.actions.PushNtfy",
                         sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionPushNtfy>),
                         &OpenWorkflowActionT<WorkflowActionPushNtfy>>);
    m_state->lua.require("porla.actions.Sleep",
                         sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionSleep>),
                         &OpenWorkflowActionT<WorkflowActionSleep>>);
    m_state->lua.require("porla.actions.TorrentMove",
                         sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionTorrentMove>),
                         &OpenWorkflowActionT<WorkflowActionTorrentMove>>);

    for (const auto& file : fs::directory_iterator(opts.workflow_dir))
    {
        if (file.path().extension() != ".lua") continue;

        try
        {
            auto workflow = m_state->lua.script_file(file.path());
            m_state->workflows.emplace_back(workflow);
        }
        catch (const sol::error& err)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to load workflow " << file.path() << ": " << err.what();
        }
    }

    m_state->on_torrent_added = opts.session.OnTorrentAdded([this](auto && s) { OnTorrentAdded(s); });
}

WorkflowEngine::~WorkflowEngine() = default;

void WorkflowEngine::OnTorrentAdded(const libtorrent::torrent_status& ts)
{
    for (const auto& instance : m_state->workflows)
    {
        Workflow* w = instance.as<Workflow*>();

        if (w->On() != "TorrentAdded")
        {
            continue;
        }

        sol::table ctx = m_state->lua.create_table();
        ctx["lt:torrent_handle"] = ts.handle;

        const WorkflowRunnerOptions opts{
            .io      = m_opts.io,
            .session = m_opts.session
        };

        BOOST_LOG_TRIVIAL(info) << "Launching workflow";

        std::make_shared<WorkflowRunner>(opts, ctx, w->Actions())->Run();
    }
}
