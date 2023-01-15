#include "ntfy.hpp"

#include <boost/log/trivial.hpp>

#include "../../../httpclient.hpp"

using porla::HttpClient;
using porla::Workflows::ActionCallback;
using porla::Workflows::Actions::Push::Ntfy;

Ntfy::Ntfy(boost::asio::io_context& io)
    : m_io(io)
{
}

void Ntfy::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    if (!params.Input().contains("topic"))
    {
        BOOST_LOG_TRIVIAL(error) << "(push/ntfy-sh) Missing required key 'topic'";
        return;
    }

    if (!params.Input().contains("message"))
    {
        BOOST_LOG_TRIVIAL(error) << "(push/ntfy-sh) Missing required key 'message'";
        return;
    }

    const auto topic   = params.Input()["topic"].get<std::string>();
    const auto message = params.Input()["message"].get<std::string>();

    auto request = HttpClient::Request{
        .url    = "https://ntfy.sh/" + topic,
        .method = "POST",
        .body   = params.Render(message)
    };

    auto http_client = std::make_shared<HttpClient>(m_io);
    http_client->SendAsync(
        request,
        [callback]()
        {
            callback->Complete({});
        });
}
