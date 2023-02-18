#include "workflowactiontorrentmove.hpp"

#include "../workflows/actions/torrentmove.hpp"

using porla::Lua::UserTypes::WorkflowActionTorrentMove;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;
using porla::Lua::Workflows::Actions::TorrentMove;
using porla::Lua::Workflows::Actions::TorrentMoveOptions;

WorkflowActionTorrentMove::WorkflowActionTorrentMove(sol::table args)
    : m_args(std::move(args))
{
    const auto path = m_args["path"];

    if (!path.is<std::string>() && !path.is<std::function<std::string(sol::table)>>())
    {
        throw std::runtime_error("Expected string or function value in TorrentMove:path parameter");
    }
}

std::shared_ptr<Action> WorkflowActionTorrentMove::Build(const ActionBuilderOptions& opts)
{
    const TorrentMoveOptions move_options{
        .path    = m_args["path"],
        .session = opts.session
    };

    return std::make_shared<TorrentMove>(move_options);
}
