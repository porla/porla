#include "sleep.hpp"

#include <boost/asio/deadline_timer.hpp>
#include <boost/log/trivial.hpp>

#include "actioncallback.hpp"

using porla::Actions::Sleep;

Sleep::Sleep(boost::asio::io_context& io)
    : m_io(io)
{
}

void Sleep::Invoke(const libtorrent::info_hash_t& hash, const toml::array& args, const std::shared_ptr<ActionCallback>& callback)
{
    if (args.empty() || !args.at(0).is_number())
    {
        return;
    }

    auto milliseconds = *args.at(0).value<int>();
    auto timer = std::make_shared<boost::asio::deadline_timer>(m_io);

    boost::system::error_code ec;
    timer->expires_from_now(boost::posix_time::milliseconds(milliseconds));

    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "(sleep) Failed to set timer expiry: " << ec.message();
    }

    timer->async_wait(
        [timer, callback](const boost::system::error_code& ec)
        {
            if (ec)
            {
                BOOST_LOG_TRIVIAL(error) << "(sleep) Error when waiting for timer: " << ec.message();
                return callback->Invoke(false);
            }

            callback->Invoke(true);
        });
}
