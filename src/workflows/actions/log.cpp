#include "log.hpp"

#include <boost/log/trivial.hpp>

using porla::Workflows::Actions::Log;

Log::Log() = default;

void Log::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    nlohmann::json output;

    if (params.Input().contains("message"))
    {
        const auto rendered_message = params.Render(params.Input()["message"]);
        BOOST_LOG_TRIVIAL(info) << rendered_message;
        output["rendered_message"] = rendered_message;
    }

    callback->Complete(output);
}
