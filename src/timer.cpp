#include "timer.hpp"

#include <boost/log/trivial.hpp>

using porla::Timer;

Timer::Timer(boost::asio::io_context& io, int interval, std::function<void()> cb)
    : m_timer(io)
    , m_interval(interval)
    , m_callback(std::move(cb))
{
    m_timer.expires_after(std::chrono::milliseconds(m_interval));
    m_timer.async_wait([this](auto &&PH1) { OnExpired(std::forward<decltype(PH1)>(PH1)); });
}

Timer::Timer(Timer&& t) noexcept
    : m_timer(std::move(t.m_timer))
    , m_interval(std::exchange(t.m_interval, 0))
    , m_callback(std::move(t.m_callback))
{
    m_timer.cancel();
    m_timer.expires_after(std::chrono::milliseconds(m_interval));
    m_timer.async_wait([this](auto &&PH1) { OnExpired(std::forward<decltype(PH1)>(PH1)); });
}

Timer::~Timer()
{
    m_timer.cancel();
}

void Timer::OnExpired(boost::system::error_code ec)
{
    if (ec == boost::system::errc::operation_canceled)
    {
        return;
    }
    else if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Error in timer: " << ec.message();
        return;
    }

    m_callback();

    m_timer.expires_after(std::chrono::milliseconds(m_interval));
    m_timer.async_wait([this](auto &&PH1) { OnExpired(std::forward<decltype(PH1)>(PH1)); });
}
