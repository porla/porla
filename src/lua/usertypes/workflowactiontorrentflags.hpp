#pragma once

#include <sol/sol.hpp>

#include "../workflows/actionbuilder.hpp"

namespace porla::Lua::UserTypes
{
    struct WorkflowActionTorrentFlags : public Workflows::ActionBuilder
    {
        static std::string ShortName() { return "TorrentFlags"; }

        explicit WorkflowActionTorrentFlags(sol::table args);
        std::shared_ptr<Workflows::Action> Build(const Workflows::ActionBuilderOptions& opts) override;

    private:
        std::vector<std::string> m_set;
        std::vector<std::string> m_unset;
    };
}
