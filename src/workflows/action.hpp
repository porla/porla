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
        [[nodiscard]] virtual nlohmann::json Input() const = 0;
        [[nodiscard]] virtual nlohmann::json Render(const std::string& text, bool raw_expression = false) const = 0;
    };

    class Action
    {
    public:
        virtual void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) = 0;
    };
}
