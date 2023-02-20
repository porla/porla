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
    std::string rendered_message;

    if (message.is<std::string>())
    {
        rendered_message = message.get<std::string>();
    }
    else if (message.is<std::function<std::string(sol::table)>>())
    {
        rendered_message = message.get<std::function<std::string(sol::table)>>()(params.context);
    }

    BOOST_LOG_TRIVIAL(info) << rendered_message;

    sol::table output          = params.state.create_table();
    output["rendered_message"] = rendered_message;

    callback->Complete(output);
}
