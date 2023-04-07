#include "sleep.hpp"

#include <boost/log/trivial.hpp>

using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::Actions::Sleep;
using porla::Lua::Workflows::Actions::SleepOptions;

Sleep::Sleep(const SleepOptions& opts)
    : m_opts(opts)
{
}

void Sleep::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    auto timer = std::make_shared<boost::asio::deadline_timer>(m_opts.io);
    timer->expires_from_now(boost::posix_time::milliseconds(m_opts.timeout));
    timer->async_wait(
        [timer, callback](const boost::system::error_code& ec)
        {
            BOOST_LOG_TRIVIAL(debug) << "(actions/sleep) Done waiting";
            callback->Complete();
        });
}
