#pragma once

#include <sol/sol.hpp>

#include "../workflows/actionbuilder.hpp"

namespace porla::Lua::UserTypes
{
    struct WorkflowActionTorrentRemove : public Workflows::ActionBuilder
    {
        static std::string ShortName() { return "TorrentRemove"; }

        explicit WorkflowActionTorrentRemove(sol::table args);
        std::shared_ptr<Workflows::Action> Build(const Workflows::ActionBuilderOptions& opts) override;

    private:
        bool m_remove_files;
    };
}
