#include "workflowactionpushntfy.hpp"

#include "../workflows/actions/pushntfy.hpp"

using porla::Lua::UserTypes::WorkflowActionPushNtfy;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;
using porla::Lua::Workflows::Actions::PushNtfy;
using porla::Lua::Workflows::Actions::PushNtfyOptions;

WorkflowActionPushNtfy::WorkflowActionPushNtfy(sol::table args)
    : m_args(std::move(args))
{
    if (!m_args["topic"].is<std::string>())
    {
        throw std::runtime_error("Expected string value in PushNtfy:topic parameter");
    }

    const auto message = m_args["message"];

    if (!message.is<std::string>() && !message.is<std::function<std::string(sol::table)>>())
    {
        throw std::runtime_error("Expected string value in PushNtfy:message parameter");
    }
}

std::shared_ptr<Action> WorkflowActionPushNtfy::Build(const ActionBuilderOptions& opts)
{
    const PushNtfyOptions options{
        .io      = opts.io,
        .topic   = m_args["topic"],
        .message = m_args["message"]
    };

    return std::make_shared<PushNtfy>(options);
}
