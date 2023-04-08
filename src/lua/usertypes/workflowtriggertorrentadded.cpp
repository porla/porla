#include "workflowtriggertorrentadded.hpp"

#include "../workflows/trigger.hpp"
#include "../workflows/triggers/torrentadded.hpp"

using porla::Lua::UserTypes::WorkflowTriggerTorrentAdded;
using porla::Lua::Workflows::Trigger;
using porla::Lua::Workflows::Triggers::TorrentAdded;
using porla::Lua::Workflows::Triggers::TorrentAddedOptions;

WorkflowTriggerTorrentAdded::WorkflowTriggerTorrentAdded(sol::table args)
    : m_args(std::move(args))
{
}

std::shared_ptr<Trigger> WorkflowTriggerTorrentAdded::Build(const Workflows::TriggerBuilderOptions &opts)
{
    const TorrentAddedOptions interval_opts{
        .actions  = opts.actions,
        .io       = opts.io,
        .lua      = opts.lua,
        .session  = opts.session
    };

    return std::make_shared<TorrentAdded>(interval_opts);
}
