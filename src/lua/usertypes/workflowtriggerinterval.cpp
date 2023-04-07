#include "workflowtriggerinterval.hpp"

#include "../workflows/trigger.hpp"
#include "../workflows/triggers/interval.hpp"

using porla::Lua::UserTypes::WorkflowTriggerInterval;
using porla::Lua::Workflows::Trigger;
using porla::Lua::Workflows::Triggers::Interval;
using porla::Lua::Workflows::Triggers::IntervalOptions;

WorkflowTriggerInterval::WorkflowTriggerInterval(sol::table args)
    : m_args(std::move(args))
{
    if (!m_args["interval"].is<int>())
    {
        throw std::runtime_error("Expected int value in Interval:interval parameter");
    }
}

std::shared_ptr<Trigger> WorkflowTriggerInterval::Build(const Workflows::TriggerBuilderOptions &opts)
{
    const IntervalOptions interval_opts{
        .actions  = opts.actions,
        .interval = m_args["interval"],
        .io       = opts.io,
        .lua      = opts.lua,
        .session  = opts.session
    };

    return std::make_shared<Interval>(interval_opts);
}
