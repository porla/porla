#pragma once

#include "../action.hpp"

namespace porla::Lua::Workflows::Actions
{
    class Log : public porla::Lua::Workflows::Action
    {
    public:
        explicit Log(sol::table args);

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        sol::table m_args;
    };
}
