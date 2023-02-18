#include "workflowactiontorrentremove.hpp"

#include "../workflows/actions/torrentremove.hpp"

using porla::Lua::UserTypes::WorkflowActionTorrentRemove;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;
using porla::Lua::Workflows::Actions::TorrentRemove;
using porla::Lua::Workflows::Actions::TorrentRemoveOptions;

WorkflowActionTorrentRemove::WorkflowActionTorrentRemove(sol::table args)
    : m_remove_files(false)
{
    if (args["remove_files"].is<bool>())
    {
        m_remove_files = args["remove_files"];
    }
}

std::shared_ptr<Action> WorkflowActionTorrentRemove::Build(const ActionBuilderOptions& opts)
{
    const TorrentRemoveOptions move_options{
        .session      = opts.session,
        .remove_files = m_remove_files,
    };

    return std::make_shared<TorrentRemove>(move_options);
}
