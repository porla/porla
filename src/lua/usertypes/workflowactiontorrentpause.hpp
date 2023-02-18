#pragma once

#include <sol/sol.hpp>

#include "../workflows/actionbuilder.hpp"

namespace porla::Lua::UserTypes
{
    struct WorkflowActionTorrentPause : public Workflows::ActionBuilder
    {
        static std::string ShortName() { return "TorrentPause"; }

        explicit WorkflowActionTorrentPause(sol::table args);
        std::shared_ptr<Workflows::Action> Build(const Workflows::ActionBuilderOptions& opts) override;
    };
}
