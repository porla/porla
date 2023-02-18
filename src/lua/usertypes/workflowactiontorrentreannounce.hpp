#pragma once

#include <sol/sol.hpp>

#include "../workflows/actionbuilder.hpp"

namespace porla::Lua::UserTypes
{
    struct WorkflowActionTorrentReannounce : public Workflows::ActionBuilder
    {
        static std::string ShortName() { return "TorrentReannounce"; }

        explicit WorkflowActionTorrentReannounce(sol::table args);
        std::shared_ptr<Workflows::Action> Build(const Workflows::ActionBuilderOptions& opts) override;

    private:
        int m_max_retries;
        int m_timeout;
    };
}
