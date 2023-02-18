#include "workflowactiontorrentpause.hpp"

#include "../workflows/actions/torrentpause.hpp"

using porla::Lua::UserTypes::WorkflowActionTorrentPause;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;
using porla::Lua::Workflows::Actions::TorrentPause;
using porla::Lua::Workflows::Actions::TorrentPauseOptions;

WorkflowActionTorrentPause::WorkflowActionTorrentPause(sol::table args)
{
}

std::shared_ptr<Action> WorkflowActionTorrentPause::Build(const ActionBuilderOptions& opts)
{
    const TorrentPauseOptions move_options{
        .session = opts.session
    };

    return std::make_shared<TorrentPause>(move_options);
}
