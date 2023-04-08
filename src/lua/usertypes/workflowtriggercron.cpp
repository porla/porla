#include "workflowtriggercron.hpp"

#include "../../query/pql.hpp"
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

    if (m_args["query"] && !m_args["query"].is<std::string>())
    {
        throw std::runtime_error("Expected string value in Cron:query parameter");
    }

    if (m_args["query"])
    {
        try
        {
            porla::Query::PQL::Parse(m_args["query"].get<std::string>());
        }
        catch (const porla::Query::QueryError& err)
        {
            std::stringstream ss;
            ss << "Failed to parse Cron:query as PQL: " << err.what() << " (pos " << err.pos() << ")";
            throw std::runtime_error(ss.str());
        }
    }
}

std::shared_ptr<Trigger> WorkflowTriggerCron::Build(const Workflows::TriggerBuilderOptions &opts)
{
    const CronOptions interval_opts{
        .actions    = opts.actions,
        .expression = m_args["expression"],
        .io         = opts.io,
        .lua        = opts.lua,
        .query      = m_args["query"],
        .session    = opts.session
    };

    return std::make_shared<Cron>(interval_opts);
}