#include "log.hpp"

#include <boost/log/trivial.hpp>
#include <utility>

using porla::Lua::Workflows::Actions::Log;

Log::Log(sol::table args)
    : m_args(std::move(args))
{
}

void Log::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    auto message = m_args["message"];

    if (message.is<std::string>())
    {
        BOOST_LOG_TRIVIAL(info) << message.get<std::string>();
    }
    else if (message.is<std::function<std::string(sol::table)>>())
    {
        BOOST_LOG_TRIVIAL(info) << message.get<std::function<std::string(sol::table)>>()(params.context);
    }

    callback->Complete();
}
