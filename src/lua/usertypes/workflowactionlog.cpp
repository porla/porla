#include "workflowactionlog.hpp"

#include "../workflows/actions/log.hpp"

using porla::Lua::UserTypes::WorkflowActionLog;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;

WorkflowActionLog::WorkflowActionLog(sol::table args)
    : m_args(std::move(args))
{
    const auto message = m_args["message"];

    if (!message.is<std::string>() && !message.is<std::function<std::string(sol::table)>>())
    {
        throw std::runtime_error("Expected string value in Log:message parameter");
    }
}

std::shared_ptr<Action> WorkflowActionLog::Build(const ActionBuilderOptions& opts)
{
    return std::make_shared<porla::Lua::Workflows::Actions::Log>(m_args);
}
