#pragma once

#include <sol/sol.hpp>

#include "../workflows/actionbuilder.hpp"

namespace porla::Lua::UserTypes
{
    struct WorkflowActionLog : public Workflows::ActionBuilder
    {
        static std::string ShortName() { return "Log"; }

        explicit WorkflowActionLog(sol::table args);
        std::shared_ptr<Workflows::Action> Build(const Workflows::ActionBuilderOptions& opts) override;

    private:
        sol::table m_args;
    };
}
