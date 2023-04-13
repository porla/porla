#include "workflowtriggertorrentmoved.hpp"

#include "../workflows/trigger.hpp"
#include "../workflows/triggers/torrentmoved.hpp"

using porla::Lua::UserTypes::WorkflowTriggerTorrentMoved;
using porla::Lua::Workflows::Trigger;
using porla::Lua::Workflows::Triggers::TorrentMoved;
using porla::Lua::Workflows::Triggers::TorrentMovedOptions;

WorkflowTriggerTorrentMoved::WorkflowTriggerTorrentMoved(sol::table args)
    : m_args(std::move(args))
{
}

std::shared_ptr<Trigger> WorkflowTriggerTorrentMoved::Build(const Workflows::TriggerBuilderOptions &opts)
{
    const TorrentMovedOptions interval_opts{
        .actions  = opts.actions,
        .filter   = opts.filter,
        .io       = opts.io,
        .lua      = opts.lua,
        .session  = opts.session
    };

    return std::make_shared<TorrentMoved>(interval_opts);
}
