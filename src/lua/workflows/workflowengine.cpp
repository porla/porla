#include "workflowengine.hpp"

#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>

#include "../usertypes/workflow.hpp"
#include "../usertypes/workflowactiontorrentflags.hpp"
#include "../usertypes/workflowactiontorrentreannounce.hpp"

#include "action.hpp"
#include "actionbuilder.hpp"
#include "triggerbuilder.hpp"

#include "../../session.hpp"

namespace fs = std::filesystem;

using porla::Lua::Workflows::WorkflowEngine;
using porla::Lua::Workflows::WorkflowEngineOptions;
using porla::Lua::UserTypes::Workflow;

using porla::Lua::UserTypes::WorkflowActionTorrentFlags;
using porla::Lua::UserTypes::WorkflowActionTorrentReannounce;

using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilder;
using porla::Lua::Workflows::ActionBuilderOptions;
using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::TriggerBuilder;
using porla::Lua::Workflows::TriggerBuilderOptions;

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

        m_lua.require("porla.Workflow", sol::c_call<decltype(&Workflow::Require), &Workflow::Require>);

        m_lua.require("porla.actions.TorrentFlags",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionTorrentFlags>),
                                 &OpenWorkflowActionT<WorkflowActionTorrentFlags>>);
        m_lua.require("porla.actions.TorrentReannounce",
                             sol::c_call<decltype(&OpenWorkflowActionT<WorkflowActionTorrentReannounce>),
                                 &OpenWorkflowActionT<WorkflowActionTorrentReannounce>>);

        if (!opts.workflow_dir.empty() && fs::exists(opts.workflow_dir))
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
