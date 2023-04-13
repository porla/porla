#pragma once

#include <sol/sol.hpp>

#include "../workflows/triggerbuilder.hpp"

namespace porla::Lua::UserTypes
{
    struct WorkflowTriggerTorrentMoved : public Workflows::TriggerBuilder
    {
        static std::string ShortName() { return "TorrentMoved"; }

        explicit WorkflowTriggerTorrentMoved(sol::table args);
        std::shared_ptr<Workflows::Trigger> Build(const Workflows::TriggerBuilderOptions& opts) override;

    private:
        sol::table m_args;
    };
}
