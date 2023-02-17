#include "pushntfy.hpp"

#include "../../../httpclient.hpp"

using porla::HttpClient;
using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::ActionParams;
using porla::Lua::Workflows::Actions::PushNtfy;
using porla::Lua::Workflows::Actions::PushNtfyOptions;

PushNtfy::PushNtfy(const PushNtfyOptions& opts)
    : m_opts(opts)
{
}

void PushNtfy::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    std::string message;

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
