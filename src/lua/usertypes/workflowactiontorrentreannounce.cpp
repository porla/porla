#include "workflowactiontorrentreannounce.hpp"

#include "../workflows/actions/torrentreannounce.hpp"

using porla::Lua::UserTypes::WorkflowActionTorrentReannounce;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;
using porla::Lua::Workflows::Actions::TorrentReannounce;
using porla::Lua::Workflows::Actions::TorrentReannounceOptions;

WorkflowActionTorrentReannounce::WorkflowActionTorrentReannounce(sol::table args)
    : m_max_retries(50)
    , m_timeout(5)
{
    if (args["max_retries"].is<int>())
    {
        m_max_retries = args["max_retries"];
    }

    if (args["timeout"].is<int>())
    {
        m_timeout = args["timeout"];
    }
}

std::shared_ptr<Action> WorkflowActionTorrentReannounce::Build(const ActionBuilderOptions& opts)
{
    const TorrentReannounceOptions move_options{
        .max_tries = m_max_retries,
        .session   = opts.session,
        .timeout   = m_timeout
    };

    return std::make_shared<TorrentReannounce>(move_options);
}
