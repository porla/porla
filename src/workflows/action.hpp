#pragma once

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

namespace porla::Workflows
{
    struct ActionCallback
    {
        virtual void Complete(const nlohmann::json& output) = 0;
    };

    struct ActionParams
    {
        virtual nlohmann::json Input() const = 0;
        virtual std::string RenderValues(const std::string& text) const = 0;
    };

    class Action
    {
    public:
        virtual void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) = 0;
    };
}
