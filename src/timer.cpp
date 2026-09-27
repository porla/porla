#include "timer.hpp"

#include <boost/log/trivial.hpp>

using porla::Timer;

std::shared_ptr<Timer> Timer::Create(boost::asio::io_context& io, int interval, std::function<void()> cb)
{
    auto t = std::shared_ptr<Timer>(new Timer(io, interval, std::move(cb)));
    t->Arm();
    return t;
}

Timer::Timer(boost::asio::io_context& io, int interval, std::function<void()> cb)
    : m_cancelled(false)
    , m_timer(io)
    , m_interval(interval)
    , m_callback(std::move(cb))
{
}

Timer::~Timer()
{
    m_cancelled = true;
    m_timer.cancel();
}

void Timer::Cancel()
{
    m_cancelled = true;
    m_timer.cancel();
}

void Timer::Arm()
{
    m_timer.expires_after(std::chrono::milliseconds(m_interval));
    m_timer.async_wait(
        [weak = weak_from_this()](boost::system::error_code ec)
        {
            if (auto self = weak.lock()) { self->OnExpired(ec); }
        });
}

void Timer::OnExpired(boost::system::error_code ec)
{
    if (m_cancelled || ec == boost::asio::error::operation_aborted)
    {
        return;
    }
    else if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Error in timer: " << ec.message();
        return;
    }

    Arm();

    try
    {
        m_callback();
    }
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(error) << "Unhandled exception in timer callback: " << ex.what();
    }
    catch (...)
    {
        BOOST_LOG_TRIVIAL(error) << "Unknown exception in timer callback";
    }
}
