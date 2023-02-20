#include "workflowengine.hpp"

#include <boost/asio.hpp>
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

#include "action.hpp"
#include "actionbuilder.hpp"
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

        for (const auto& file : fs::directory_iterator(opts.workflow_dir))
        {
            if (file.path().extension() != ".lua") continue;

            try
            {
                auto workflow = m_lua.script_file(file.path());
                m_workflows.emplace_back(workflow);
            }
            catch (const sol::error& err)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to load workflow " << file.path() << ": " << err.what();
            }
        }

        m_on_torrent_added    = m_opts.session.OnTorrentAdded([this](auto && s) { OnTorrentAdded(s); });
        m_on_torrent_finished = m_opts.session.OnTorrentFinished([this](auto && s) { OnTorrentFinished(s); });
    }

    ~State()
    {
        m_on_torrent_added.disconnect();
        m_on_torrent_finished.disconnect();
    }

private:
    void OnTorrentAdded(const libtorrent::torrent_status& ts)
    {
        sol::table ctx           = m_lua.create_table();
        ctx["lt:torrent_handle"] = ts.handle;
        ctx["torrent"]           = Torrent{ts.handle};

        RunEvents("TorrentAdded", ctx);
    }

    void OnTorrentFinished(const libtorrent::torrent_status& ts)
    {
        sol::table ctx           = m_lua.create_table();
        ctx["lt:torrent_handle"] = ts.handle;
        ctx["torrent"]           = Torrent{ts.handle};

        RunEvents("TorrentFinished", ctx);
    }

    void RunEvents(const std::string& eventName, sol::table& ctx)
    {
        ctx["actions"] = std::vector<sol::object>();

        for (const auto& instance : m_workflows)
        {
            Workflow* w = instance.as<Workflow*>();

            if (w->On() != eventName)
            {
                continue;
            }

            if (!w->ShouldExecute(ctx))
            {
                continue;
            }

            const WorkflowRunnerOptions opts{
                .io      = m_opts.io,
                .lua     = m_lua,
                .session = m_opts.session
            };

            BOOST_LOG_TRIVIAL(info) << "Launching workflow";

            auto workflow = std::make_shared<WorkflowRunner>(opts, ctx, w->Actions());

            boost::asio::post(
                m_opts.io,
                [workflow]() { workflow->Run(); });
        }
    }

    sol::state                  m_lua;
    boost::signals2::connection m_on_torrent_added;
    boost::signals2::connection m_on_torrent_finished;
    WorkflowEngineOptions       m_opts;
    std::vector<sol::object>    m_workflows;
};

WorkflowEngine::WorkflowEngine(const WorkflowEngineOptions& opts)
    : m_state(std::make_unique<State>(opts))
{
}

WorkflowEngine::~WorkflowEngine() = default;
