#include "workflowactiontorrentmove.hpp"



using porla::Lua::UserTypes::WorkflowActionTorrentMove;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;

WorkflowActionTorrentMove::WorkflowActionTorrentMove(sol::table args)
    : m_args(std::move(args))
{
}

std::shared_ptr<Action> WorkflowActionTorrentMove::Build(const ActionBuilderOptions& opts)
{
    return nullptr;
}
