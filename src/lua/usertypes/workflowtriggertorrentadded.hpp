#pragma once

#include <sol/sol.hpp>

#include "../workflows/triggerbuilder.hpp"

namespace porla::Lua::UserTypes
{
    struct WorkflowTriggerTorrentAdded : public Workflows::TriggerBuilder
    {
        static std::string ShortName() { return "TorrentAdded"; }

        explicit WorkflowTriggerTorrentAdded(sol::table args);
        std::shared_ptr<Workflows::Trigger> Build(const Workflows::TriggerBuilderOptions& opts) override;

    private:
        sol::table m_args;
    };
}
