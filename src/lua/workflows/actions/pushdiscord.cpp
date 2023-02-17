#include "pushdiscord.hpp"

#include <nlohmann/json.hpp>

#include "../../../httpclient.hpp"

using porla::HttpClient;
using porla::Lua::Workflows::Actions::PushDiscord;
using porla::Lua::Workflows::Actions::PushDiscordOptions;

PushDiscord::PushDiscord(const PushDiscordOptions& opts)
    : m_opts(opts)
{
}

void PushDiscord::Invoke(const ActionParams &params, std::shared_ptr<ActionCallback> callback)
{
    std::string content;

    if (m_opts.message.is<std::string>())
    {
        content = m_opts.message.as<std::string>();
    }
    else if (m_opts.message.is<std::function<std::string(sol::table)>>())
    {
        content = m_opts.message.as<std::function<std::string(sol::table)>>()(params.context);
    }

    const HttpClient::Request request{
        .url    = m_opts.url,
        .method = "POST",
        .body   = nlohmann::json({
            {"content", content}
        }).dump()
    };

    auto http_client = std::make_shared<HttpClient>(m_opts.io);
    http_client->SendAsync(
        request,
        [http_client, callback]()
        {
            callback->Complete();
        });
}
