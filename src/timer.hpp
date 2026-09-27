#pragma once

#include <functional>
#include <memory>

#include <boost/asio.hpp>

namespace porla
{
    class Timer : public std::enable_shared_from_this<Timer>
    {
    public:
        static std::shared_ptr<Timer> Create(
            boost::asio::io_context& io, int interval, std::function<void()> cb);

        ~Timer();

        Timer(const Timer&) = delete;
        Timer(Timer&&) = delete;

        Timer& operator=(const Timer&) = delete;
        Timer& operator=(Timer&&) = delete;

        void Cancel();

    private:
        explicit Timer(boost::asio::io_context& io, int interval, std::function<void()> cb);
        void Arm();
        void OnExpired(boost::system::error_code ec);

        bool                      m_cancelled;
        boost::asio::steady_timer m_timer;
        int                       m_interval;
        std::function<void()>     m_callback;
    };
}
