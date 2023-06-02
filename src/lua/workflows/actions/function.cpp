#include "function.hpp"

#include <utility>

using porla::Lua::Workflows::Actions::Function;

Function::Function(sol::table args)
    : m_args(std::move(args))
{
}

void Function::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    auto func = m_args["function"];

    if (func.is<sol::function>())
    {
        func(params.context);
    }

    callback->Complete();
}
