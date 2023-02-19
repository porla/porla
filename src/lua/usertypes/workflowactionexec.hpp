#pragma once

#include <sol/sol.hpp>

#include "../workflows/actionbuilder.hpp"

namespace porla::Lua::UserTypes
{
    struct WorkflowActionExec : public Workflows::ActionBuilder
    {
        static std::string ShortName() { return "Exec"; }

        explicit WorkflowActionExec(sol::table args);
        std::shared_ptr<Workflows::Action> Build(const Workflows::ActionBuilderOptions& opts) override;

    private:
        sol::table m_args;
    };
}
