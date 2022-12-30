#include "sleep.hpp"

#include <boost/log/trivial.hpp>

using porla::Workflows::ActionCallback;
using porla::Workflows::Actions::Sleep;

Sleep::Sleep(boost::asio::io_context& io)
    : m_io(io)
{
}

void Sleep::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    BOOST_LOG_TRIVIAL(debug) << "(actions/sleep): Invoking";

    const auto timeout = params.Input().at("timeout").get<int>();

    auto timer = std::make_shared<boost::asio::deadline_timer>(m_io);
    timer->expires_from_now(boost::posix_time::milliseconds(timeout));
    timer->async_wait(
        [timer, callback](const boost::system::error_code& ec)
        {
            BOOST_LOG_TRIVIAL(debug) << "(actions/sleep): Completing";
            callback->Complete({});
        });
}
