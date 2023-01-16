#include "http.hpp"

#include "../../httpclient.hpp"

using porla::HttpClient;
using porla::Workflows::Actions::Http;

Http::Http(boost::asio::io_context &io)
    : m_io(io)
{
}

void Http::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    const auto url = params.Input()["url"].get<std::string>();
    const auto body = params.Input()["body"].get<std::string>();

    const auto request = HttpClient::Request{
        .url    = url,
        .method = "POST",
        .body   = params.Render(body).dump()
    };

    auto http_client = std::make_shared<HttpClient>(m_io);
    http_client->SendAsync(
        request,
        [http_client, callback]()
        {
            callback->Complete({});
        });
}
