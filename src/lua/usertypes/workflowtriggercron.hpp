#pragma once

#include <sol/sol.hpp>

#include "../workflows/triggerbuilder.hpp"

namespace porla::Lua::UserTypes
{
    struct WorkflowTriggerCron : public Workflows::TriggerBuilder
    {
        static std::string ShortName() { return "Cron"; }

        explicit WorkflowTriggerCron(sol::table args);
        std::shared_ptr<Workflows::Trigger> Build(const Workflows::TriggerBuilderOptions& opts) override;

    private:
        sol::table m_args;
    };
}
