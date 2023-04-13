#include "workflowengine.hpp"

#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>

#include "../usertypes/lttorrenthandle.hpp"
#include "../usertypes/torrent.hpp"
#include "../usertypes/workflow.hpp"
#include "../usertypes/workflowactionexec.hpp"
#include "../usertypes/workflowactionlog.hpp"
#include "../usertypes/workflowactionpushdiscord.hpp"
#include "../usertypes/workflowactionpushntfy.hpp"
#include "../usertypes/workflowactionsleep.hpp"
#include "../usertypes/workflowactiontorrentflags.hpp"
#include "../usertypes/workflowactiontorrentmove.hpp"
#include "../usertypes/workflowactiontorrentpause.hpp"
#include "../usertypes/workflowactiontorrentreannounce.hpp"
#include "../usertypes/workflowactiontorrentremove.hpp"
#include "../usertypes/workflowtriggercron.hpp"
#include "../usertypes/workflowtriggerinterval.hpp"
#include "../usertypes/workflowtriggertorrentadded.hpp"
#include "../usertypes/workflowtriggertorrentfinished.hpp"
#include "../usertypes/workflowtriggertorrentmoved.hpp"

#include "action.hpp"
#include "actionbuilder.hpp"
#include "triggerbuilder.hpp"
#include "workflowrunner.hpp"

#include "../../session.hpp"

namespace fs = std::filesystem;

using porla::Lua::Workflows::WorkflowEngine;
using porla::Lua::Workflows::WorkflowEngineOptions;
using porla::Lua::UserTypes::Workflow;

using porla::Lua::UserTypes::LibtorrentTorrentHandle;
using porla::Lua::UserTypes::Torrent;
using porla::Lua::UserTypes::WorkflowActionExec;
using porla::Lua::UserTypes::WorkflowActionLog;
using porla::Lua::UserTypes::WorkflowActionPushDiscord;
using porla::Lua::UserTypes::WorkflowActionPushNtfy;
using porla::Lua::UserTypes::WorkflowActionSleep;
using porla::Lua::UserTypes::WorkflowActionTorrentFlags;
using porla::Lua::UserTypes::WorkflowActionTorrentMove;
using porla::Lua::UserTypes::WorkflowActionTorrentPause;
using porla::Lua::UserTypes::WorkflowActionTorrentReannounce;
using porla::Lua::UserTypes::WorkflowActionTorrentRemove;
using porla::Lua::UserTypes::WorkflowTriggerCron;
using porla::Lua::UserTypes::WorkflowTriggerInterval;
using porla::Lua::UserTypes::WorkflowTriggerTorrentAdded;
using porla::Lua::UserTypes::WorkflowTriggerTorrentFinished;
using porla::Lua::UserTypes::WorkflowTriggerTorrentMoved;

using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilder;
using porla::Lua::Workflows::ActionBuilderOptions;
using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::TriggerBuilder;
using porla::Lua::Workflows::TriggerBuilderOptions;
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

template<typename T>
sol::table OpenWorkflowTriggerT(sol::this_state s)
{
    sol::state_view lua(s);

    sol::table module = lua.create_table();
    module.new_usertype<T>(
            T::ShortName(),
            sol::constructors<T(sol::table)>(),
            sol::base_classes,
            sol::bases<TriggerBuilder>());

    return module[T::ShortName()];
}

class WorkflowEngine::State
{
public:
    explicit State(const WorkflowEngineOptions& opts)
        : m_opts(opts)
    {
        m_lua.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::string,
            sol::lib::table);

        LibtorrentTorrentHandle::Register(m_lua);
        Torrent::Register(m_lua);

        m_lua.require("porla.Workflow", sol::c_call<decltype(&Workflow::Require), &Workflow::Require>);

        m_lua.require("porla.actions.Exec",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionExec>),
                                 &OpenWorkflowActionT<WorkflowActionExec>>);
        m_lua.require("porla.actions.Log",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionLog>),
                                 &OpenWorkflowActionT<WorkflowActionLog>>);
        m_lua.require("porla.actions.PushDiscord",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionPushDiscord>),
                                 &OpenWorkflowActionT<WorkflowActionPushDiscord>>);
        m_lua.require("porla.actions.PushNtfy",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionPushNtfy>),
                                 &OpenWorkflowActionT<WorkflowActionPushNtfy>>);
        m_lua.require("porla.actions.Sleep",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionSleep>),
                                 &OpenWorkflowActionT<WorkflowActionSleep>>);
        m_lua.require("porla.actions.TorrentFlags",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionTorrentFlags>),
                                 &OpenWorkflowActionT<WorkflowActionTorrentFlags>>);
        m_lua.require("porla.actions.TorrentMove",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionTorrentMove>),
                                 &OpenWorkflowActionT<WorkflowActionTorrentMove>>);
        m_lua.require("porla.actions.TorrentPause",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionTorrentPause>),
                                 &OpenWorkflowActionT<WorkflowActionTorrentPause>>);
        m_lua.require("porla.actions.TorrentReannounce",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionTorrentReannounce>),
                                 &OpenWorkflowActionT<WorkflowActionTorrentReannounce>>);
        m_lua.require("porla.actions.TorrentRemove",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionTorrentRemove>),
                                 &OpenWorkflowActionT<WorkflowActionTorrentRemove>>);

        // triggers
        m_lua.require("porla.triggers.Cron",
            sol::c_call<decltype(&OpenWorkflowTriggerT<WorkflowTriggerCron>),
                &OpenWorkflowTriggerT<WorkflowTriggerCron>>);

        m_lua.require("porla.triggers.Interval",
            sol::c_call<decltype(&OpenWorkflowTriggerT<WorkflowTriggerInterval>),
                &OpenWorkflowTriggerT<WorkflowTriggerInterval>>);

        m_lua.require("porla.triggers.TorrentAdded",
            sol::c_call<decltype(&OpenWorkflowTriggerT<WorkflowTriggerTorrentAdded>),
                &OpenWorkflowTriggerT<WorkflowTriggerTorrentAdded>>);

        m_lua.require("porla.triggers.TorrentFinished",
            sol::c_call<decltype(&OpenWorkflowTriggerT<WorkflowTriggerTorrentFinished>),
                &OpenWorkflowTriggerT<WorkflowTriggerTorrentFinished>>);

        m_lua.require("porla.triggers.TorrentMoved",
            sol::c_call<decltype(&OpenWorkflowTriggerT<WorkflowTriggerTorrentMoved>),
                &OpenWorkflowTriggerT<WorkflowTriggerTorrentMoved>>);

        if (!opts.workflow_dir.empty())
        {
            for (const auto &file: fs::directory_iterator(opts.workflow_dir))
            {
                if (file.path().extension() != ".lua") continue;

                try
                {
                    auto result = m_lua.script_file(file.path());
                    auto workflow = result.get<Workflow*>();

                    auto& trigger_builder = workflow->TriggerBuilder();
                    auto trigger = trigger_builder.Build(TriggerBuilderOptions{
                        .actions = workflow->Actions(),
                        .filter  = workflow->Filter(),
                        .io      = opts.io,
                        .lua     = m_lua,
                        .session = opts.session
                    });

                    m_workflow_triggers.emplace_back(trigger);
                }
                catch (const sol::error &err)
                {
                    BOOST_LOG_TRIVIAL(error) << "Failed to load workflow " << file.path() << ": " << err.what();
                }
            }
        }
    }

private:
    sol::state                            m_lua;
    WorkflowEngineOptions                 m_opts;
    std::vector<std::shared_ptr<Trigger>> m_workflow_triggers;
};

WorkflowEngine::WorkflowEngine(const WorkflowEngineOptions& opts)
    : m_state(std::make_unique<State>(opts))
{
}

WorkflowEngine::~WorkflowEngine() = default;
