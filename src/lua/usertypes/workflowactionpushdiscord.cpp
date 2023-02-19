#include "workflowactionpushdiscord.hpp"

#include "../workflows/actions/pushdiscord.hpp"

using porla::Lua::UserTypes::WorkflowActionPushDiscord;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;
using porla::Lua::Workflows::Actions::PushDiscord;
using porla::Lua::Workflows::Actions::PushDiscordOptions;

WorkflowActionPushDiscord::WorkflowActionPushDiscord(sol::table args)
    : m_args(std::move(args))
{
    if (!m_args["url"].is<std::string>())
    {
        throw std::runtime_error("Expected string value in PushDiscord:url parameter");
    }

    const auto message = m_args["message"];

    if (!message.is<std::string>() && !message.is<std::function<std::string(sol::table)>>())
    {
        throw std::runtime_error("Expected string value in PushDiscord:message parameter");
    }
}

std::shared_ptr<Action> WorkflowActionPushDiscord::Build(const ActionBuilderOptions& opts)
{
    const PushDiscordOptions options{
        .io      = opts.io,
        .url     = m_args["url"],
        .message = m_args["message"]
    };

    return std::make_shared<PushDiscord>(options);
}
