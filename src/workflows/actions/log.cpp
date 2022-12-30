#include "log.hpp"

#include <boost/log/trivial.hpp>

using porla::Workflows::Actions::Log;

Log::Log() = default;

void Log::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    if (params.Input().contains("message"))
    {
        BOOST_LOG_TRIVIAL(info) << params.RenderValues(params.Input()["message"]);
    }

    callback->Complete({});
}
