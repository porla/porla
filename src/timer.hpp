#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <boost/asio.hpp>

namespace porla
{
    class Timer : public std::enable_shared_from_this<Timer>
    {
    public:
        static std::shared_ptr<Timer> Cron(
            boost::asio::io_context& io, const std::string& expression, std::function<void()> cb);

        static std::shared_ptr<Timer> Interval(
            boost::asio::io_context& io, std::chrono::milliseconds interval, std::function<void()> cb);

        ~Timer();

        Timer(const Timer&) = delete;
        Timer(Timer&&) = delete;

        Timer& operator=(const Timer&) = delete;
        Timer& operator=(Timer&&) = delete;

        void Cancel();

    private:
        using NextDelay = std::function<std::optional<std::chrono::milliseconds>()>;

        static std::shared_ptr<Timer> Start(boost::asio::io_context& io, NextDelay next, std::function<void()> cb);

        explicit Timer(boost::asio::io_context& io, NextDelay next, std::function<void()> cb);

        void Arm();
        void OnExpired(boost::system::error_code ec);

        bool                      m_cancelled;
        boost::asio::steady_timer m_timer;
        NextDelay                 m_next;
        std::function<void()>     m_callback;
    };
}
