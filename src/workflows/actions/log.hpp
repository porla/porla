#pragma once

#include "../action.hpp"

namespace porla::Workflows::Actions
{
    class Log : public porla::Workflows::Action
    {
    public:
        explicit Log();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;
    };
}
