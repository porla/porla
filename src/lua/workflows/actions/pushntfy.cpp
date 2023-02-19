#include "pushntfy.hpp"

#include <utility>

#include "../../../httpclient.hpp"

using porla::HttpClient;
using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::ActionParams;
using porla::Lua::Workflows::Actions::PushNtfy;
using porla::Lua::Workflows::Actions::PushNtfyOptions;

PushNtfy::PushNtfy(PushNtfyOptions opts)
    : m_opts(std::move(opts))
{
}

void PushNtfy::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    std::string message;

    if (m_opts.message.is<std::string>())
    {
        message = m_opts.message.as<std::string>();
    }
    else if (m_opts.message.is<std::function<std::string(sol::table)>>())
    {
        message = m_opts.message.as<std::function<std::string(sol::table)>>()(params.context);
    }

    const HttpClient::Request request{
        .url    = "https://ntfy.sh/" + m_opts.topic,
        .method = "POST",
        .body   = message
    };

    auto http_client = std::make_shared<HttpClient>(m_opts.io);
    http_client->SendAsync(
        request,
        [http_client, callback]()
        {
            callback->Complete();
        });
}
