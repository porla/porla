#pragma once

#include <functional>
#include <boost/asio.hpp>

namespace porla
{
    class Timer
    {
    public:
        explicit Timer(boost::asio::io_context& io, int interval, std::function<void()> cb);
        Timer(Timer&& t) noexcept;
        ~Timer();

        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;
        Timer& operator=(Timer&&) = delete; // noexcept {}

    private:
        void OnExpired(boost::system::error_code ec);

        boost::asio::deadline_timer m_timer;
        int                         m_interval;
        std::function<void()>       m_callback;
    };
}
