#include "workflowtriggercron.hpp"

#include "../workflows/trigger.hpp"
#include "../workflows/triggers/cron.hpp"

#include <croncpp.hpp>

using porla::Lua::UserTypes::WorkflowTriggerCron;
using porla::Lua::Workflows::Trigger;
using porla::Lua::Workflows::Triggers::Cron;
using porla::Lua::Workflows::Triggers::CronOptions;

WorkflowTriggerCron::WorkflowTriggerCron(sol::table args)
    : m_args(std::move(args))
{
    if (!m_args["expression"].is<std::string>())
    {
        throw std::runtime_error("Expected string value in Cron:expression parameter");
    }
}

std::shared_ptr<Trigger> WorkflowTriggerCron::Build(const Workflows::TriggerBuilderOptions &opts)
{
    const CronOptions interval_opts{
        .actions    = opts.actions,
        .expression = m_args["expression"],
        .io         = opts.io,
        .lua        = opts.lua,
        .session    = opts.session
    };

    return std::make_shared<Cron>(interval_opts);
}
