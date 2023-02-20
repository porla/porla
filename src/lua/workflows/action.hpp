#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Workflows
{
    struct ActionCallback
    {
        virtual void Complete(sol::object output) = 0;
        virtual void Complete() = 0;
    };

    struct ActionParams
    {
        sol::table  context;
        sol::state& state;
    };

    class Action
    {
    public:
        virtual void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) = 0;
    };
}
