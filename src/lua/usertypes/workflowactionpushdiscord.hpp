#pragma once

#include <sol/sol.hpp>

#include "../workflows/actionbuilder.hpp"

namespace porla::Lua::UserTypes
{
    struct WorkflowActionPushDiscord : public Workflows::ActionBuilder
    {
        static std::string ShortName() { return "PushDiscord"; }

        explicit WorkflowActionPushDiscord(sol::table args);
        std::shared_ptr<Workflows::Action> Build(const Workflows::ActionBuilderOptions& opts) override;

    private:
        sol::table m_args;
    };
}
