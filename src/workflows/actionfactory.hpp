#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace porla::Workflows
{
    class Action;

    class ActionFactory
    {
    public:
        explicit ActionFactory(const std::map<std::string, std::function<std::shared_ptr<Action>()>>& actions);
        ~ActionFactory();

        std::shared_ptr<Action> Construct(const std::string& action_name) const;

    private:
        std::map<std::string, std::function<std::shared_ptr<Action>()>> m_actions;
    };
}
