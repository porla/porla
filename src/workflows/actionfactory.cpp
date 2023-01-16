#include "actionfactory.hpp"

#include "action.hpp"

using porla::Workflows::Action;
using porla::Workflows::ActionFactory;

ActionFactory::ActionFactory(const std::map<std::string, std::function<std::shared_ptr<Action>()>>& actions)
    : m_actions(actions)
{
}

ActionFactory::~ActionFactory() = default;

std::shared_ptr<Action> ActionFactory::Construct(const std::string &action_name) const
{
    if (m_actions.contains(action_name))
    {
        return m_actions.at(action_name)();
    }

    return nullptr;
}
