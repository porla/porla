#pragma once

#include "../action.hpp"

namespace porla::Lua::Workflows::Actions
{
    class Function : public porla::Lua::Workflows::Action
    {
    public:
        explicit Function(sol::table args);

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        sol::table m_args;
    };
}
