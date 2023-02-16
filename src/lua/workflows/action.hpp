#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Workflows
{
    struct ActionCallback
    {
        virtual void Complete() = 0;
    };

    struct ActionParams
    {
        sol::table context;
    };

    class Action
    {
    public:
        virtual void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) = 0;
    };
}
