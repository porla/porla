#include "workflowactionsleep.hpp"

#include "../workflows/actions/sleep.hpp"

using porla::Lua::UserTypes::WorkflowActionSleep;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;
using porla::Lua::Workflows::Actions::Sleep;
using porla::Lua::Workflows::Actions::SleepOptions;

WorkflowActionSleep::WorkflowActionSleep(sol::table args)
    : m_args(std::move(args))
{
    if (!m_args["timeout"].is<int>())
    {
        throw std::runtime_error("Expected int value in Sleep:timeout parameter");
    }
}

std::shared_ptr<Action> WorkflowActionSleep::Build(const ActionBuilderOptions& opts)
{
    const SleepOptions options{
        .io      = opts.io,
        .timeout = m_args["timeout"]
    };

    return std::make_shared<Sleep>(options);
}
