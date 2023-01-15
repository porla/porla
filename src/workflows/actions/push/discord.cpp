#include "discord.hpp"

#include <boost/log/trivial.hpp>

#include "../../../httpclient.hpp"

using porla::HttpClient;
using porla::Workflows::Actions::Push::Discord;

Discord::Discord(boost::asio::io_context &io)
    : m_io(io)
{
}

void Discord::Invoke(const ActionParams &params, std::shared_ptr<ActionCallback> callback)
{
    if (!params.Input().contains("url"))
    {
        BOOST_LOG_TRIVIAL(error) << "(push/discord) Missing required key 'url'";
        return;
    }

    if (!params.Input().contains("message"))
    {
        BOOST_LOG_TRIVIAL(error) << "(push/discord) Missing required key 'message'";
        return;
    }

    const auto url     = params.Input()["url"].get<std::string>();
    const auto message = params.Input()["message"].get<std::string>();

    auto request = HttpClient::Request{
        .url    = url,
        .method = "POST",
        .body   = nlohmann::json({
            {"content", params.Render(message)}
        }).dump()
    };

    auto http_client = std::make_shared<HttpClient>(m_io);
    http_client->SendAsync(
        request,
        [callback]()
        {
            callback->Complete({});
        });
}
