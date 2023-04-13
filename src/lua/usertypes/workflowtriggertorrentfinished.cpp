#include "workflowtriggertorrentfinished.hpp"

#include "../workflows/trigger.hpp"
#include "../workflows/triggers/torrentfinished.hpp"

using porla::Lua::UserTypes::WorkflowTriggerTorrentFinished;
using porla::Lua::Workflows::Trigger;
using porla::Lua::Workflows::Triggers::TorrentFinished;
using porla::Lua::Workflows::Triggers::TorrentFinishedOptions;

WorkflowTriggerTorrentFinished::WorkflowTriggerTorrentFinished(sol::table args)
    : m_args(std::move(args))
{
}

std::shared_ptr<Trigger> WorkflowTriggerTorrentFinished::Build(const Workflows::TriggerBuilderOptions &opts)
{
    const TorrentFinishedOptions interval_opts{
        .actions  = opts.actions,
        .filter   = opts.filter,
        .io       = opts.io,
        .lua      = opts.lua,
        .session  = opts.session
    };

    return std::make_shared<TorrentFinished>(interval_opts);
}
